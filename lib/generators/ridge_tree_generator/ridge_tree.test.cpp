//@	{"target":{"name":"./ridge_tree.test"}}

#include "./ridge_tree.hpp"

#include "lib/curve_tools/dump.hpp"
#include "lib/common/cfile_owner.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/image_io.hpp"
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
					.damping = std::sqrt(0.5f)
				},
				.growth_params{
					.max_length = terraformer::domain_length{49152.0f},
					.min_neighbour_distance = terraformer::domain_length{49152.0f}
				}
			},
			terraformer::ridge_tree_branch_description{
				.displacement_profile {
					.amplitude = terraformer::horizontal_amplitude{3096.0f/3.0f},
					.wavelength = terraformer::domain_length{12384.0f/3.0f},
					.damping = std::sqrt(0.5f)
				},
				.growth_params{
					.max_length = terraformer::domain_length{12384.0f},
					.min_neighbour_distance = terraformer::domain_length{6144.0f}
				}
			},
			terraformer::ridge_tree_branch_description{
				.displacement_profile {
					.amplitude = terraformer::horizontal_amplitude{3096.0f/9.0f},
					.wavelength = terraformer::domain_length{12384.0f/9.0f},
					.damping = std::sqrt(0.5f)
				},
				.growth_params{
					.max_length = terraformer::domain_length{12384.0f/3.0f},
					.min_neighbour_distance = terraformer::domain_length{1536.0f}
				}
			},
			terraformer::ridge_tree_branch_description{
				.displacement_profile {
					.amplitude = terraformer::horizontal_amplitude{3096.0f/27.0f},
					.wavelength = terraformer::domain_length{12384.0f/27.0f},
					.damping = std::sqrt(0.5f)
				},
				.growth_params {
					.max_length = terraformer::domain_length{12384.0f/9.0f},
					.min_neighbour_distance = terraformer::domain_length{384.0f}
				}
			}
		}
	};

	constexpr auto pixel_size = 48.0f;
	auto const t_0 = std::chrono::steady_clock::now();
	auto res = generate(
		desc,
		rng,
		pixel_size
	);
	auto const t = std::chrono::steady_clock::now();
	printf("Elapsed time %.8g s\n", std::chrono::duration<double>(t - t_0).count());

	terraformer::curve_set curves;
	for(auto k = res.first_element_index(); k != std::size(res); ++k)
	{
		for(auto const& curve : res[k].curves)
		{
			curves.append(curve.points());
		}
	}

	auto curve_file = terraformer::make_output_file("/dev/shm/slask.json");
	curves.write_to(curve_file.get());
#if 0
	std::vector elevation_profiles{
		terraformer::ridge_tree_branch_elevation_profile{
			.starting_slope = terraformer::slope_angle{0.0f},
			.final_elevation = terraformer::elevation{3072.0f},
			.final_slope = terraformer::slope_angle{0.0f},
			.starting_peak_displacement = terraformer::vertical_amplitude{512.0f},
			.final_peak_displacement = terraformer::vertical_amplitude{512.0f},
			.min_peak_angle = terraformer::slope_angle{0.0f},
			.max_peak_angle = terraformer::slope_angle{0.0f},
			.noise = terraformer::ridge_tree_elevation_noise{
				.amplitude = terraformer::vertical_amplitude{64.0f},
				.wavelength = terraformer::domain_length{256.0f},
				.damping = std::sqrt(0.5f)
			}
		},
		terraformer::ridge_tree_branch_elevation_profile{
			.starting_slope = terraformer::slope_angle{0.0f},
			.final_elevation = terraformer::elevation{256.0f},
			.final_slope = terraformer::slope_angle{0.0f},
			.starting_peak_displacement = terraformer::vertical_amplitude{512.0f},
			.final_peak_displacement = terraformer::vertical_amplitude{43.0f},
			.min_peak_angle = terraformer::slope_angle{0.0f},
			.max_peak_angle = terraformer::slope_angle{0.0f},
			.noise = terraformer::ridge_tree_elevation_noise{
				.amplitude = terraformer::vertical_amplitude{16.0f},
				.wavelength = terraformer::domain_length{256.0f},
				.damping = std::sqrt(0.5f)
			}
		},
		terraformer::ridge_tree_branch_elevation_profile{
			.starting_slope = terraformer::slope_angle{0.0f},
			.final_elevation = terraformer::elevation{128.0f},
			.final_slope = terraformer::slope_angle{0.0f},
			.starting_peak_displacement = terraformer::vertical_amplitude{64.0f},
			.final_peak_displacement = terraformer::vertical_amplitude{16.0f},
			.min_peak_angle = terraformer::slope_angle{0.0f},
			.max_peak_angle = terraformer::slope_angle{0.0f},
			.noise = terraformer::ridge_tree_elevation_noise{
				.amplitude = terraformer::vertical_amplitude{16.0f},
				.wavelength = terraformer::domain_length{256.0f},
				.damping = std::sqrt(0.5f)
			}
		},
		terraformer::ridge_tree_branch_elevation_profile{
			.starting_slope = terraformer::slope_angle{0.0f},
			.final_elevation = terraformer::elevation{64.0f},
			.final_slope = terraformer::slope_angle{0.0f},
			.starting_peak_displacement = terraformer::vertical_amplitude{16.0f},
			.final_peak_displacement = terraformer::vertical_amplitude{8.0f},
			.min_peak_angle = terraformer::slope_angle{0.0f},
			.max_peak_angle = terraformer::slope_angle{0.0f},
			.noise = terraformer::ridge_tree_elevation_noise{
				.amplitude = terraformer::vertical_amplitude{16.0f},
				.wavelength = terraformer::domain_length{256.0f},
				.damping = std::sqrt(0.5f)
			}
		}
	};

	res.update_elevations(
 		terraformer::elevation{2048.0f},
		elevation_profiles,
		rng,
		pixel_size
	);

	terraformer::grayscale_image img{1024, 1024};
	render(
		res,
		img,
		terraformer::ridge_tree_render_description{
			.curve_levels = std::vector{
				terraformer::ridge_tree_branch_render_description{
					.peak_elevation = 3500.0f,
					.peak_radius = 3500.0f,
				},
				terraformer::ridge_tree_branch_render_description{
					.peak_elevation = 2000.0f,
					.peak_radius = 2000.0f,
				},
				terraformer::ridge_tree_branch_render_description{
					.peak_elevation = 1000.0f,
					.peak_radius = 1000.0f,
				},
				terraformer::ridge_tree_branch_render_description{
					.peak_elevation = 1000.0f,
					.peak_radius = 300.0f,
				},
			}
		},
		pixel_size
	);
	store(img, "/dev/shm/slask.exr");
#endif
}