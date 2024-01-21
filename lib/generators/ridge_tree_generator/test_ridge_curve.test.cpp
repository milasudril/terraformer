//@ {"target":{"name":"test_ridge_curve"}}

#include "./ridge_curve.hpp"
#include "./ridge_tree.hpp"
#include "./ridge_tree_branch_seed_sequence.hpp"

#include "lib/curve_tools/length.hpp"
#include "lib/curve_tools/displace.hpp"
#include "lib/curve_tools/dump.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include "lib/math_utils/differentiation.hpp"
#include "lib/common/array_tuple.hpp"
#include "lib/common/cfile_owner.hpp"
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

	terraformer::curve_set curves;

	curves.append(root.get<0>());

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
	{ curves.append(branch.get<0>()); }

	for(auto const& branch: right_siblings)
 	{ curves.append(branch.get<0>()); }

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
	{ curves.append(branch.get<0>()); }

	for(auto const& branch: next_level_right)
	{ curves.append(branch.get<0>()); }

	auto curve_file = terraformer::make_output_file("/dev/shm/slask.json");
	curves.write_to(curve_file.get());

	store(potential, "test.exr");
}
