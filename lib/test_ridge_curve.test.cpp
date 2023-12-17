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
			// Note: Add a small disturbance to y coordinate to avoid singular values when computing
			//       the curve normal
			ret[k] = start_loc
				+ displacement{static_cast<float>(k)*dx, (static_cast<float>(k%2) - 0.5f)*dx/1024.0f, 0.0f};
		}
		return ret;
	}

	template<class BranchStopCondition>
	std::vector<location> generate_branch_base_curve(
		terraformer::location loc,
		terraformer::direction start_dir,
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
			loc -= pixel_size*direction{
				grad(
					potential,
					loc[0]/pixel_size,
					loc[1]/pixel_size,
					1.0f,
					clamp_at_boundary{}
				)
			};
		}
		return base_curve;
	}

	std::vector<ridge_tree_branch>
	generate_branches(
		terraformer::ridge_tree_branch::seed_info const& seeds,
		span_2d<float const> potential,
		float pixel_size,
		ridge_curve_description curve_desc,
		random_generator& rng,
		std::vector<ridge_tree_branch>&& existing_branches = std::vector<ridge_tree_branch>{})
	{
		auto const points = seeds.branch_points.get<0>();
		auto const normals = seeds.branch_points.get<1>();
		for(size_t k = 0; k != std::size(seeds.branch_points); ++k)
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

#if 0
	std::vector<std::vector<location>>
	generate_branches(std::span<ridge_tree_branch const> trees,
		span_2d<float const> potential,
		float pixel_size
	)
	{
		auto branches = generate_branches(
			trees[0].left_seeds().branch_points,
			potential,
			pixel_size
		);

		for(size_t k = 1; k != std::size(trees); ++k)
		{
			branches = generate_branches(
				trees[k - 1].right_seeds().branch_points,
				potential,
				pixel_size,
				std::move(branches)
			);

			branches = generate_branches(
				trees[k].left_seeds().branch_points,
				potential,
				pixel_size,
				std::move(branches)
			);
		}

		return branches;
	}
#endif
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
	store(potential, "test1.exr");

	terraformer::ridge_curve_description const curve_desc_2{
		.amplitude = terraformer::horizontal_amplitude{3096.0f/3.0f},
		.wavelength = terraformer::domain_length{12384.0f/3.0f},
		.damping = std::sqrt(0.5f),
		.flip_direction = false,
		.invert_displacement = false
	};

	auto const branches = generate_branches(
		root.left_seeds(),
		potential,
		pixel_size,
		curve_desc_2,
		rng,
		generate_branches(root.right_seeds(), potential, pixel_size, curve_desc_2, rng)
	);

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

	store(potential, "test.exr");
}