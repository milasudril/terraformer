//@ {"target":{"name":"test_ridge_curve"}}

#include "./ridge_curve.hpp"
#include "./boundary_sampling_policies.hpp"
#include "./curve_length.hpp"
#include "./curve_displace.hpp"
#include "./tempdir.hpp"
#include "./ridge_tree_branch_seed_sequence.hpp"

#include "lib/common/array_tuple.hpp"
#include "lib/pixel_store/image_io.hpp"

#include <random>

namespace terraformer
{
	void dump_curve(std::span<location const> points, std::filesystem::path const& output_name)
	{
		std::unique_ptr<FILE, decltype(&fclose)> dest{fopen(output_name.c_str(), "wb"), fclose};
		for(size_t k = 0; k != std::size(points); ++k)
		{
			auto const loc = points[k];
			fprintf(dest.get(), "%.8g %.8g %.8g\n", loc[0], loc[1], loc[2]);
		}
	}

	std::vector<location> make_point_array(location start_loc, size_t count, float dx)
	{
		std::vector<location> ret(count);
		for(size_t k = 0; k != count; ++k)
		{
			ret[k] = start_loc
				+ displacement{static_cast<float>(k)*dx, 0.0f, 0.0f};
		}
		return ret;
	}

	template<class BranchStopCondition>
	std::vector<location> generate_branch_base_curve(
		location loc,
		direction start_dir,
		span_2d<float const> potential,
		float pixel_size,
		BranchStopCondition&& stop)
	{
		std::vector<location> base_curve;
		if(stop(loc) || !inside(potential, loc[0]/pixel_size, loc[1]/pixel_size))
		{ return base_curve; }

		base_curve.push_back(loc);

		loc += pixel_size*start_dir;

		while(!stop(loc) && inside(potential, loc[0]/pixel_size, loc[1]/pixel_size))
		{
			base_curve.push_back(loc);
			auto const g = direction{
				grad(
					potential,
					loc[0]/pixel_size,
					loc[1]/pixel_size,
					1.0f,
					clamp_at_boundary{}
				)
			};

			loc -= pixel_size*g;
		}
		return base_curve;
	}

	using ridge_tree_branch = array_tuple<location, float>;

	std::vector<ridge_tree_branch>
	generate_branches(
		array_tuple<location, direction> const& branch_points,
		span_2d<float const> potential,
		float pixel_size,
		ridge_curve_description curve_desc,
		random_generator& rng,
		float d_max,
		std::vector<ridge_tree_branch>&& existing_branches = std::vector<ridge_tree_branch>{})
	{
		auto const points = branch_points.get<0>();
		auto const normals = branch_points.get<1>();
		for(size_t k = 0; k != std::size(branch_points); ++k)
		{
			auto const base_curve = generate_branch_base_curve(
				points[k],
				normals[k],
				potential,
				pixel_size,
				[d = 0.0f, loc_prev = points[k], d_max](auto loc) mutable {
					auto new_distance = d + distance(loc, loc_prev);
					if(new_distance > d_max)
					{ return true; }
					d = new_distance;
					loc_prev = loc;
					return false;
				}
			);

			if(std::size(base_curve) < 3)
			{
				fprintf(stderr, "Curve %zu is to short\n", k);
				continue;
			}

			auto const base_curve_length = static_cast<size_t>(curve_length(base_curve)/pixel_size) + 1;
			auto const offsets = generate(curve_desc, rng, base_curve_length, pixel_size);

			existing_branches.push_back(
				displace_xy(
					base_curve,
					displacement_profile{
						.offsets = offsets,
						.sample_period = pixel_size,
					}
				)
			);
		}

		return existing_branches;
	}
	void trim_at_intersect(std::vector<ridge_tree_branch>& a, std::vector<ridge_tree_branch>& b, float threshold)
	{
		auto const outer_count = std::size(a);
		auto const inner_count = std::size(b);

		std::vector<size_t> a_trim(outer_count);
		for(size_t k = 0; k != outer_count; ++k)
		{ a_trim[k] = std::size(a[k]); }

		std::vector<size_t> b_trim(inner_count);
		for(size_t l = 0; l != inner_count; ++l)
		{ b_trim[l] = std::size(b[l]); }

		for(size_t k = 0; k != outer_count; ++k)
		{
			for(size_t l = 0; l != inner_count; ++l)
			{
				auto const res = cartesian_find_if(
					std::span<location const>(a[k].get<0>()),
					std::span<location const>(b[l].get<0>()),
					[threshold](auto const p1, auto const p2) {
						if(distance(p1, p2) < threshold)
						{ return true; }
						return false;
					}
				);

				a_trim[k] = std::min(res.first, a_trim[k]);
				b_trim[l] = std::min(res.second, b_trim[l]);
			}
		}

		for(size_t k = 0; k != outer_count; ++k)
		{
			for(size_t l = 0; l != k; ++l)
			{
				auto const res = cartesian_find_if(
					std::span<location const>(a[k].get<0>()),
					std::span<location const>(a[l].get<0>()),
					[threshold](auto const p1, auto const p2) {
						if(distance(p1, p2) < threshold)
						{ return true; }
						return false;
					}
				);

				a_trim[k] = std::min(res.first, b_trim[k]);
				a_trim[l] = std::min(res.second, b_trim[l]);
			}
		}

		for(size_t k = 0; k != inner_count; ++k)
		{
			for(size_t l = 0; l != k; ++l)
			{
				auto const res = cartesian_find_if(
					std::span<location const>(b[k].get<0>()),
					std::span<location const>(b[l].get<0>()),
					[threshold](auto const p1, auto const p2) {
						if(distance(p1, p2) < threshold)
						{ return true; }
						return false;
					}
				);

				b_trim[k] = std::min(res.first, b_trim[k]);
				b_trim[l] = std::min(res.second, b_trim[l]);
			}
		}

		for(size_t k = 0; k != outer_count; ++k)
		{
			auto const index = a_trim[k];
			if(index != std::size(a[k]))
			{ a[k].shrink(index); }
		}

		for(size_t l = 0; l != inner_count; ++l)
		{
			auto const index = b_trim[l];
			if(index != std::size(b[l]))
			{	b[l].shrink(index); }
		}
	}

	std::vector<ridge_tree_branch>
	generate_branches(
		std::span<ridge_tree_branch_seed_sequence const> parents,
		span_2d<float const> potential,
		float pixel_size,
		ridge_curve_description curve_desc,
		random_generator& rng,
		float max_length
	)
	{
		if(std::size(parents) == 0)
		{	return std::vector<ridge_tree_branch>{}; }

		auto output_branches = generate_branches(
			parents[0].left,
			potential,
			pixel_size,
			curve_desc,
			rng,
			max_length
		);

		for(size_t k = 1; k != std::size(parents); ++k)
		{
			auto right_branches = generate_branches(
				parents[k - 1].right,
				potential,
				pixel_size,
				curve_desc,
				rng,
				max_length
			);

			auto left_branches = generate_branches(
				parents[k].left,
				potential,
				pixel_size,
				curve_desc,
				rng,
				max_length
			);

			trim_at_intersect(right_branches, left_branches, 1536.0f);

			std::ranges::transform(std::move(right_branches), std::back_inserter(output_branches), [](auto&& val){
				return std::move(val);
			});

			std::ranges::transform(std::move(left_branches), std::back_inserter(output_branches), [](auto&& val){
				return std::move(val);
			});
		}

		output_branches = generate_branches(
			parents.back().right,
			potential,
			pixel_size,
			curve_desc,
			rng,
			max_length,
			std::move(output_branches)
		);

		return output_branches;
	}

	float compute_potential(std::span<ridge_tree_branch const> branches, location r, float min_distance)
	{
		auto sum = 0.0f;
		for(size_t k = 0; k != std::size(branches); ++k)
		{
			auto const points = branches[k].get<0>();
			sum += terraformer::fold_over_line_segments(
				points,
				[](auto seg, auto point, auto min_distance, auto... prev) {
					auto const d = distance(seg, point);
					auto const l = length(seg);
					return (prev + ... + (l*(d<min_distance? 1.0f : min_distance/d)));
				},
				r,
				min_distance
			);
		}
		return sum;
	}
}

int main()
{
	terraformer::ridge_curve_description curve_desc{
		.amplitude = terraformer::horizontal_amplitude{3096.0f},
		.wavelength = terraformer::domain_length{12384.0f},
		.damping = std::sqrt(0.5f),
		.flip_direction = false,
		.invert_displacement = false
	};

	constexpr auto pixel_size = 48.0f;
	terraformer::random_generator rng;
	auto const pixel_count = static_cast<size_t>(49152.0f/pixel_size);
	auto const offsets = generate(
		curve_desc,
		rng,
		pixel_count,
		pixel_size,
		1024.0f);
	auto const ridge_loc = 24576.0f;
	auto const curve = terraformer::make_point_array(terraformer::location{0.0f, ridge_loc, 0.0f}, pixel_count, pixel_size);

	auto const root = displace_xy(
		curve,
		terraformer::displacement_profile{
			.offsets = offsets,
			.sample_period = pixel_size
		}
	);

	terraformer::tempdir dir{"/dev/shm/test_ridge_curve_XXXXXX"};
	auto const dirname = dir.keep_after_scope(true).get_name();

	size_t curve_count = 0;
	terraformer::dump_curve(root.get<0>(), dirname / std::to_string(curve_count).append(".txt"));
	++curve_count;

	terraformer::grayscale_image potential{pixel_count, pixel_count};
	{
		auto const points = root.get<0>();
		for(uint32_t y = 0; y != potential.height(); ++y)
		{
			for(uint32_t x = 0; x != potential.width(); ++x)
			{
				terraformer::location const loc_xy{pixel_size*static_cast<float>(x), pixel_size*static_cast<float>(y), 0.0f};

				potential(x, y) = terraformer::fold_over_line_segments(
					points,
					[loc_xy](auto seg, auto point, auto... prev) {
						auto const d1 = distance(seg, point + terraformer::displacement{-49152.0f,0.0f,0.0f});
						auto const d2 = distance(seg, point);
						auto const d3 = distance(seg, point + terraformer::displacement{49152.0f,0.0f,0.0f});
						auto const d = std::min(d1, std::min(d2, d3));
						auto const l = length(seg);
						auto const d_min = 0.5f*pixel_size;
						return (prev + ... + (l*(d<d_min? 1.0f : d_min/d)));
					},
					loc_xy
				);
			}
		}
	}

	terraformer::ridge_curve_description const curve_desc_2{
		.amplitude = terraformer::horizontal_amplitude{3096.0f/3.0f},
		.wavelength = terraformer::domain_length{12384.0f/3.0f},
		.damping = std::sqrt(0.5f),
		.flip_direction = false,
		.invert_displacement = false
	};

	auto const root_seeds = terraformer::collect_ridge_tree_branch_seeds(root);

	auto const left_siblings = generate_branches(
		root_seeds.left,
		potential,
		pixel_size,
		curve_desc_2,
		rng,
		12384.0f
	);

	auto const right_siblings = generate_branches(
		root_seeds.right,
		potential,
		pixel_size,
		curve_desc_2,
		rng,
		12384.0f
	);

	for(auto const& branch: left_siblings)
	{
		terraformer::dump_curve(branch.get<0>(), dirname / std::to_string(curve_count).append(".txt"));
		++curve_count;
	}

	for(auto const& branch: right_siblings)
	{
		terraformer::dump_curve(branch.get<0>(), dirname / std::to_string(curve_count).append(".txt"));
		++curve_count;
	}

	for(uint32_t y = 0; y != potential.height(); ++y)
	{
		for(uint32_t x = 0; x != potential.width(); ++x)
		{
			auto sum = 0.0f;
			terraformer::location const loc_xy{
				pixel_size*static_cast<float>(x),
				pixel_size*static_cast<float>(y),
				0.0f
			};

			sum += terraformer::compute_potential(left_siblings, loc_xy, 0.5f*pixel_size);
			sum += terraformer::compute_potential(right_siblings, loc_xy, 0.5f*pixel_size);

			potential(x, y) += sum;
		}
	}

	terraformer::ridge_curve_description const curve_desc_3{
		.amplitude = terraformer::horizontal_amplitude{3096.0f/9.0f},
		.wavelength = terraformer::domain_length{12384.0f/9.0f},
		.damping = std::sqrt(0.5f),
		.flip_direction = false,
		.invert_displacement = false
	};

	auto const next_level_left_seeds = terraformer::collect_ridge_tree_branch_seeds(left_siblings);
	auto const next_level_left = generate_branches(
		next_level_left_seeds,
		potential,
		pixel_size,
		curve_desc_3,
		rng,
		24576.0f
	);

	auto const next_level_right_seeds = terraformer::collect_ridge_tree_branch_seeds(right_siblings);
	auto const next_level_right = generate_branches(
		next_level_right_seeds,
		potential,
		pixel_size,
		curve_desc_3,
		rng,
		24576.0f
	);

	for(auto const& branch: next_level_left)
	{
		terraformer::dump_curve(branch.get<0>(), dirname / std::to_string(curve_count).append(".txt"));
		++curve_count;
	}

	for(auto const& branch: next_level_right)
	{
		terraformer::dump_curve(branch.get<0>(), dirname / std::to_string(curve_count).append(".txt"));
		++curve_count;
	}

	store(potential, "test.exr");
}
