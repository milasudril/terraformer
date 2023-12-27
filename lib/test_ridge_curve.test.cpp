//@ {"target":{"name":"test_ridge_curve"}}

#include "./ridge_curve.hpp"
#include "./boundary_sampling_policies.hpp"
#include "./ridge_tree_branch.hpp"
#include "./curve_length.hpp"

#include "lib/pixel_store/image_io.hpp"

#include <random>

namespace terraformer
{
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
				printf("Curve is too short\n");
				continue;
			}

			auto const base_curve_length = static_cast<size_t>(curve_length(base_curve)/pixel_size) + 1;
			auto const offsets = generate(curve_desc, rng, base_curve_length, pixel_size);

#if 0
			if(curve_index == 10)
			{
				{
					std::string filename{"testdata/basecurve_2.dat"};
					filename.append(std::to_string(curve_index));
					auto dump = fopen(filename.c_str(),"wb");
					static_assert(std::is_same_v<decltype(std::data(base_curve)), location const*>);
					fwrite(std::data(base_curve), sizeof(location), std::size(base_curve), dump);
					fclose(dump);
				}

				{
					std::string filename{"testdata/random_curve_2.dat"};
					filename.append(std::to_string(curve_index));
					auto dump = fopen(filename.c_str(),"wb");
					static_assert(std::is_same_v<decltype(std::data(offsets)), float const*>);
					fwrite(std::data(offsets), sizeof(float), std::size(offsets), dump);
					fclose(dump);
				}
			}
#endif
			existing_branches.push_back(
				ridge_tree_branch{
					base_curve,
					displacement_profile{
						.offsets = offsets,
						.sample_period = pixel_size,
					}
				}
			);
		}

		return existing_branches;
	}

	std::vector<array_tuple<location, float>>
	generate_delimiters(
		array_tuple<location, direction> const& delimiter_points,
		span_2d<float const> potential,
		float pixel_size,
		ridge_curve_description curve_desc,
		random_generator& rng,
		std::vector<array_tuple<location, float>>&& existing_delimiters =  std::vector<array_tuple<location, float>>{})
	{
		auto const points = delimiter_points.get<0>();
		auto const normals = delimiter_points.get<1>();
		for(size_t k = 0; k != std::size(delimiter_points); ++k)
		{
			auto const base_curve = generate_branch_base_curve(
				points[k],
				normals[k],
				potential,
				pixel_size,
				[](auto...){return false;}
			);

			auto const base_curve_length = static_cast<size_t>(curve_length(base_curve)/pixel_size) + 1;
			auto const offsets = generate(curve_desc, rng, base_curve_length, pixel_size);

			existing_delimiters.push_back(
				displace_xy(
					base_curve,
					displacement_profile{
						.offsets = offsets,
						.sample_period = pixel_size,
					}
				)
			);
		}

		return existing_delimiters;
	}

	std::vector<ridge_tree_branch>
	generate_branches(
		std::span<ridge_tree_branch const> branches,
		span_2d<float const> potential,
		float pixel_size,
		ridge_curve_description curve_desc,
		random_generator& rng
	)
	{
		if(std::size(branches) == 0)
		{	return std::vector<ridge_tree_branch>{}; }

		auto output_branches = generate_branches(
			branches[0].left_seeds().branch_points,
			potential,
			pixel_size,
			curve_desc,
			rng,
			3072.0f
		);

		for(size_t k = 1; k != std::size(branches); ++k)
		{
			output_branches = generate_branches(
				branches[k - 1].right_seeds().branch_points,
				potential,
				pixel_size,
				curve_desc,
				rng,
				3072.0f,
				std::move(output_branches)
			);

			output_branches = generate_branches(
				branches[k].left_seeds().branch_points,
				potential,
				pixel_size,
				curve_desc,
				rng,
				3072.0f,
				std::move(output_branches)
			);
		}

		output_branches = generate_branches(
			branches.back().right_seeds().branch_points,
			potential,
			pixel_size,
			curve_desc,
			rng,
			3072.0f,
			std::move(output_branches)
		);

		return output_branches;
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

	constexpr auto pixel_size = 32.0f;
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

	terraformer::ridge_tree_branch root{
		curve,
		terraformer::displacement_profile{
			.offsets = offsets,
			.sample_period = pixel_size
		}
	};

	terraformer::grayscale_image potential{pixel_count, pixel_count};
	{
		auto const& root_curve = root.curve();
		auto const points = root_curve.get<0>();
		for(uint32_t y = 0; y != potential.height(); ++y)
		{
			for(uint32_t x = 0; x != potential.width(); ++x)
			{
				terraformer::location const loc_xy{pixel_size*static_cast<float>(x), pixel_size*static_cast<float>(y), 0.0f};

				auto sum = std::accumulate(std::begin(points), std::end(points), 0.0f, [loc_xy](auto const sum, auto const point) {
					auto const d1 = terraformer::distance_xy(loc_xy, point + terraformer::displacement{-49152.0f,0.0f,0.0f});
					auto const d2 = terraformer::distance_xy(loc_xy, point);
					auto const d3 = terraformer::distance_xy(loc_xy, point + terraformer::displacement{49152.0f,0.0f,0.0f});

					auto const d = std::min(std::min(d1, d2), d3);

					auto const d_min = 1.0f*pixel_size;
					return sum + 1.0f*(d<d_min? 1.0f : (d_min)/(d));
				});

				potential(x, y) = sum;
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

	auto const branches = generate_branches(
		root.left_seeds().branch_points,
		potential,
		pixel_size,
		curve_desc_2,
		rng,
		12384.0f,
		generate_branches(root.right_seeds().branch_points, potential, pixel_size, curve_desc_2, rng, 12384.0f)
	);

/*
	auto const delimiters = generate_delimiters(
		root.left_seeds().delimiter_points,
		potential,
		pixel_size,
		curve_desc_2,
		rng,
		generate_delimiters(root.right_seeds().delimiter_points, potential, pixel_size, curve_desc_2, rng)
	);
*/

	for(uint32_t y = 0; y != potential.height(); ++y)
	{
		for(uint32_t x = 0; x != potential.width(); ++x)
		{
			auto sum = 0.0f;
			for(size_t k = 0; k != std::size(branches); ++k)
			{
				auto const points = branches[k].curve().get<0>();
				terraformer::location const loc_xy{pixel_size*static_cast<float>(x), pixel_size*static_cast<float>(y), 0.0f};
				sum += std::accumulate(std::begin(points), std::end(points), 0.0f, [loc_xy](auto const sum, auto const point) {
					auto const d = terraformer::distance_xy(loc_xy, point);
					auto const d_min = 1.0f*pixel_size;
					return sum + 1.0f*(d<d_min? 1.0f : (d_min)/(d));
				});
			}

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

	auto next_level = generate_branches(branches,
		potential,
		pixel_size,
		curve_desc_3,
		rng
	);

	for(uint32_t y = 0; y != potential.height(); ++y)
	{
		for(uint32_t x = 0; x != potential.width(); ++x)
		{
			auto sum = 0.0f;
			for(size_t k = 0; k != std::size(next_level); ++k)
			{
				auto const points = next_level[k].curve().get<0>();
				terraformer::location const loc_xy{pixel_size*static_cast<float>(x), pixel_size*static_cast<float>(y), 0.0f};
				sum += std::accumulate(std::begin(points), std::end(points), 0.0f, [loc_xy](auto const sum, auto const point) {
					auto const d = terraformer::distance_xy(loc_xy, point);
					auto const d_min = 1.0f*pixel_size;
					return sum + 1.0f*(d<d_min? 1.0f : (d_min)/(d));
				});
			}

			potential(x, y) += sum;
		}
	}

	store(potential, "test.exr");
}
