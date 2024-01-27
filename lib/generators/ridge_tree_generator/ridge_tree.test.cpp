//@	{"target":{"name":"./ridge_tree.test"}}

#include "./ridge_tree.hpp"

#include "lib/curve_tools/dump.hpp"
#include "lib/common/cfile_owner.hpp"
#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ridge_tree_generate)
{
	terraformer::random_generator const rng;
	terraformer::ridge_tree_description const desc{
		.root_location = terraformer::location{0.0f, 24576.0f, 0.0f},
		.trunk_direction = terraformer::direction{terraformer::displacement{1.0f, 0.0f, 0.0f}},
		.curve_levels = std::vector{
			terraformer::ridge_tree_branch_description{
				.displacement_profile {
					.amplitude = terraformer::horizontal_amplitude{3096.0f},
					.wavelength = terraformer::domain_length{12384.0f},
					.damping = std::sqrt(0.5f),
					.flip_direction = false,
					.invert_displacement = false,
				},
				.max_length = terraformer::domain_length{49152.0f}
			},
			terraformer::ridge_tree_branch_description{
				.displacement_profile {
					.amplitude = terraformer::horizontal_amplitude{3096.0f/3.0f},
					.wavelength = terraformer::domain_length{12384.0f/3.0f},
					.damping = std::sqrt(0.5f),
					.flip_direction = false,
					.invert_displacement = false,
				},
				.max_length = terraformer::domain_length{12384.0f}
			},
			terraformer::ridge_tree_branch_description{
				.displacement_profile {
					.amplitude = terraformer::horizontal_amplitude{3096.0f/9.0f},
					.wavelength = terraformer::domain_length{12384.0f/9.0f},
					.damping = std::sqrt(0.5f),
					.flip_direction = false,
					.invert_displacement = false,
				},
				.max_length = terraformer::domain_length{6144.0f}
			},
		}
	};

	auto const res = generate(
		desc,
		rng,
		48.0f
	);

	printf("%zu\n", std::size(res));

	terraformer::curve_set curves;
	for(size_t k = 0; k != std::size(res); ++k)
	{
		for(auto const& curve : res[k].curves)
		{
			curves.append(curve.points());
		}
	}

	auto curve_file = terraformer::make_output_file("/dev/shm/slask2.json");
	curves.write_to(curve_file.get());
}