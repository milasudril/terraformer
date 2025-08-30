//@	{"target":{"name":"./ridge_tree.test"}}

#include "./ridge_tree.hpp"

#include "lib/curve_tools/dump.hpp"
#include "lib/common/cfile_owner.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/image_io.hpp"
#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ridge_tree_generate)
{
	terraformer::random_generator rng;
	terraformer::ridge_tree_xy_description const desc{
		.root_location = terraformer::location{0.0f, 24576.0f, 0.0f},
		.trunk_direction = terraformer::direction{terraformer::displacement{1.0f, 0.0f, 0.0f}},
		.trunk{
			.begin{terraformer::location{0.0f, 24576.0f, 0.0f}, terraformer::displacement{1024.0f, 0.0f, 0.0f}},
			.end{terraformer::location{49152.0f, 24576.0f, 0.0f}, terraformer::displacement{1024.0f, 0.0f, 0.0f}}
		},
		.trunk_growth_params{
			.max_length = terraformer::domain_length{49152.0f},
			.min_neighbour_distance = terraformer::domain_length{49152.0f}
		},
		.branch_growth_params = std::vector{
			terraformer::ridge_tree_branch_growth_description{
				.max_length = terraformer::domain_length{12384.0f},
				.min_neighbour_distance = terraformer::domain_length{6144.0f}
			},
			terraformer::ridge_tree_branch_growth_description{
				.max_length = terraformer::domain_length{12384.0f/3.0f},
				.min_neighbour_distance = terraformer::domain_length{1536.0f}
			},
			terraformer::ridge_tree_branch_growth_description{
				.max_length = terraformer::domain_length{12384.0f/9.0f},
				.min_neighbour_distance = terraformer::domain_length{384.0f}
			}
		},
		.displacement_profiles = std::vector{
			terraformer::ridge_tree_branch_displacement_description{
				.amplitude = terraformer::horizontal_amplitude{3096.0f},
				.wavelength = terraformer::domain_length{12384.0f},
				.damping = std::sqrt(0.5f)
			},
			terraformer::ridge_tree_branch_displacement_description{
				.amplitude = terraformer::horizontal_amplitude{3096.0f/3.0f},
				.wavelength = terraformer::domain_length{12384.0f/3.0f},
				.damping = std::sqrt(0.5f)
			},
			terraformer::ridge_tree_branch_displacement_description{
				.amplitude = terraformer::horizontal_amplitude{3096.0f/9.0f},
				.wavelength = terraformer::domain_length{12384.0f/9.0f},
				.damping = std::sqrt(0.5f)
			},
			terraformer::ridge_tree_branch_displacement_description{
				.amplitude = terraformer::horizontal_amplitude{3096.0f/27.0f},
				.wavelength = terraformer::domain_length{12384.0f/27.0f},
				.damping = std::sqrt(0.5f)
			}
		},
	};

	auto const t_0 = std::chrono::steady_clock::now();
	auto res = generate(
		desc,
		rng
	);
	auto const t = std::chrono::steady_clock::now();
	printf("Elapsed time %.8g s\n", std::chrono::duration<double>(t - t_0).count());

	for(auto& current_trunk : res)
	{
		auto const level = current_trunk.level;

		auto const parent = current_trunk.parent;
		if(parent == terraformer::ridge_tree_trunk::no_parent)
		{
			EXPECT_EQ(level, 0);
			continue;
		}

		auto const parent_curve_index = current_trunk.parent_curve_index;
		auto const parent_curves = res[parent].branches.get<0>().decay();
		auto const parent_curve = parent_curves[parent_curve_index].points();
		auto const branches = current_trunk.branches.get<0>();
		auto const start_index = current_trunk.branches.get<1>();
		auto const seed_indices = current_trunk.branches.get<2>();

		for(auto k : current_trunk.branches.element_indices())
		{
			EXPECT_LT(start_index[k].get(), std::size(parent_curve).get());
			auto const point_on_parent = parent_curve[start_index[k]];
			auto const point_on_current_curve = branches[k].points().front();

			{
				terraformer::displaced_curve::index_type prev_index{0};
				for(auto index : seed_indices[k])
				{
					EXPECT_GE(index.get(), prev_index.get());
					EXPECT_LT(index.get(), std::size(branches[k]).get());
					prev_index = index;
				}
			}

			EXPECT_EQ(point_on_parent, point_on_current_curve);
		}
	}
	terraformer::curve_set curves;
	for(auto const& item : res)
	{
		for(auto const& curve : item.branches.get<0>())
		{
			curves.append(curve.points());
		}
	}

	auto curve_file = terraformer::make_output_file("/dev/shm/slask.json");
	curves.write_to(curve_file.get());

	std::vector elevation_profiles{
		terraformer::ridge_tree_branch_elevation_profile{
			.base_elevation{
				.starting_slope{
					.min = terraformer::slope_angle{0.0f},
					.max = terraformer::slope_angle{0.0f}
				},
				.final_elevation = terraformer::elevation{2560.0f},
				.final_slope{
					.min = terraformer::slope_angle{0.0f},
					.max = terraformer::slope_angle{0.0f}
				}
			},
			.modulation{
				.mod_depth = terraformer::modulation_depth{2.0f/5.0f},
				.peak_noise_mix = terraformer::blend_value{1.0f/6.0f},
				.per_peak_modulation{
					.slope{
						.min = terraformer::slope_angle{1.0f/24.0f},
						.max = terraformer::slope_angle{1.0f/8.0f}
					}
				},
				.elevation_noise{
					.wavelength = terraformer::domain_length{2064.0f},
					.damping = std::sqrt(0.5f)
				}
			}
		}
		,
		terraformer::ridge_tree_branch_elevation_profile{
			.base_elevation{
				.starting_slope{
					.min = terraformer::slope_angle{0.0f},
					.max = terraformer::slope_angle{1.0f/24}
				},
				.final_elevation = terraformer::elevation{512.0f},
				.final_slope{
					.min = terraformer::slope_angle{0.0f},
					.max = terraformer::slope_angle{1.0f/12.0f}
				}
			},
			.modulation{
				.mod_depth = terraformer::modulation_depth{1.0f/5.0f},
				.peak_noise_mix = terraformer::blend_value{1.0f/6.0f},
				.per_peak_modulation{
					.slope{
						.min = terraformer::slope_angle{1.0f/24.0f},
						.max = terraformer::slope_angle{1.0f/8.0f}
					}
				},
				.elevation_noise{
					.wavelength = terraformer::domain_length{688.0f},
					.damping = std::sqrt(0.5f)
				}
			}
		}
		,
		terraformer::ridge_tree_branch_elevation_profile{
			.base_elevation{
				.starting_slope{
					.min = terraformer::slope_angle{0.0f},
					.max = terraformer::slope_angle{1.0f/48.0f}
				},
				.final_elevation = terraformer::elevation{229.0f},
				.final_slope{
					.min = terraformer::slope_angle{0.0f},
					.max = terraformer::slope_angle{1.0f/24.0f}
				}
			},
			.modulation{
				.mod_depth = terraformer::modulation_depth{2.0f/15.0f},
				.peak_noise_mix = terraformer::blend_value{1.0f/6.0f},
				.per_peak_modulation{
					.slope{
						.min = terraformer::slope_angle{1.0f/24.0f},
						.max = terraformer::slope_angle{1.0f/8.0f}
					}
				},
				.elevation_noise{
					.wavelength = terraformer::domain_length{229.0f},
					.damping = std::sqrt(0.5f)
				}
			}
		},
		terraformer::ridge_tree_branch_elevation_profile{
			.base_elevation{
				.starting_slope{
					.min = terraformer::slope_angle{0.0f},
					.max = terraformer::slope_angle{1.0f/12.0f}
				},
				.final_elevation = terraformer::elevation{204.0f},
				.final_slope{
					.min = terraformer::slope_angle{0.0f},
					.max = terraformer::slope_angle{0.0f}
				}
			},
			.modulation{
				.mod_depth = terraformer::modulation_depth{2.0f/25.0f},
				.peak_noise_mix = terraformer::blend_value{1.0f/6.0f},
				.per_peak_modulation{
					.slope{
						.min = terraformer::slope_angle{1.0f/24.0f},
						.max = terraformer::slope_angle{1.0f/25.0f}
					}
				},
				.elevation_noise{
					.wavelength = terraformer::domain_length{76.0f},
					.damping = std::sqrt(0.5f)
				}
			}
		}
	};

	res.update_elevations(
 		terraformer::elevation{2560.0f},
		elevation_profiles,
		rng
	);
}