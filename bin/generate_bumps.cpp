//@	{"target":{"name":"generate_bumps.o"}}

#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/image_io.hpp"
#include "lib/common/spaces.hpp"
#include "lib/filters/diffuser.hpp"
#include "lib/filters/curve_rasterizer.hpp"
#include "lib/filters/bump_field.hpp"
#include "lib/filters/bump_field_2.hpp"
#include "lib/curve_tool/fractal_wave.hpp"
#include "lib/execution/thread_pool.hpp"
#include "lib/filters/diffuser.hpp"
#include "lib/filters/waveshaper.hpp"
#include "lib/filters/convhull.hpp"

#include <random>
#include <chrono>
#include <array>
#include <pcg-cpp/include/pcg_random.hpp>
#include <cmath>

using random_generator = pcg_engines::oneseq_dxsm_128_64;

struct corner
{
	float elevation;
};

struct corners
{
	corner sw;
	corner se;
	corner nw;
	corner ne;
};

struct main_ridge_params
{
	terraformer::location start_location;
	float distance_xy_to_endpoint;
	float base_elevation;
	terraformer::fractal_wave_params ridge_curve;
	terraformer::fractal_wave_params ridge_curve_xz;
};

struct uplift_zone
{
	float radius_south;
	float radius_north;
	terraformer::fractal_wave_params radius_distortion;
};

struct steady_plate_collision_zone_descriptor
{
	struct corners corners;
	main_ridge_params main_ridge;
	struct uplift_zone uplift_zone;
	terraformer::bump_field::params bump_field;
};

int main()
{
	using namespace terraformer;

	static constexpr auto pixel_size = 48.0f;
	static constexpr uint32_t domain_width = 1024;
	static constexpr uint32_t domain_height = 1024;

	steady_plate_collision_zone_descriptor const heightmap_params{
		.corners{
			.sw = corner{512.0f},
			.se = corner{1536.0f},
			.nw = corner{3584.0f},
			.ne = corner{2560.0f}
		},
		.main_ridge{
			.start_location = terraformer::location{0.0f, 16384.0f, 3072.0f},
			.distance_xy_to_endpoint = 49152.0f,
			.base_elevation = 4096.0f,
			.ridge_curve{
				.shape{
					.amplitude{
						.scaling_factor = std::numbers::phi_v<float>,
						.scaling_noise = std::numbers::phi_v<float>/16.0f
					},
					.wavelength{
						.scaling_factor = std::numbers::phi_v<float>,
						.scaling_noise = std::numbers::phi_v<float>/16.0f
					},
					.phase{
						.offset = 2.0f - std::numbers::phi_v<float>,
						.offset_noise = 1.0f/12.0f
					}
				},
				.wave_properties{
					.wavelength = 24576.0f,
					.phase = 0.0f
				}
			},
			.ridge_curve_xz{
				.shape{
					.amplitude{
						.scaling_factor = std::numbers::phi_v<float>,
						.scaling_noise = std::numbers::phi_v<float>/16.0f
					},
					.wavelength{
						.scaling_factor = std::numbers::phi_v<float>,
						.scaling_noise = std::numbers::phi_v<float>/16.0f
					},
					.phase{
						.offset = 2.0f - std::numbers::phi_v<float>,
						.offset_noise = 1.0f/12.0f
					}
				},
				.wave_properties{
					.wavelength = 11264.0f,
					.phase = 0.0f
				}
			}
		},
		.uplift_zone{
			.radius_south = 11264.0f,
			.radius_north = 8192.0f,
			.radius_distortion{
				.shape{
					.amplitude{
						.scaling_factor = std::numbers::phi_v<float>,
						.scaling_noise = std::numbers::phi_v<float>/16.0f
					},
					.wavelength{
						.scaling_factor = std::numbers::phi_v<float>,
						.scaling_noise = std::numbers::phi_v<float>/16.0f
					},
					.phase{
						.offset = 2.0f - std::numbers::phi_v<float>,
						.offset_noise = 1.0f/12.0f
					}
				},
				.wave_properties{
					.wavelength = 2048.0f,
					.phase = 0.0f
				}
			}
		},
		.bump_field{
			.impact_waves{
				.shape{
					.amplitude{
						.scaling_factor = std::numbers::phi_v<float>,
						.scaling_noise = std::numbers::phi_v<float>/8.0f
					},
					.wavelength{
						.scaling_factor = std::numbers::phi_v<float>,
						.scaling_noise = std::numbers::phi_v<float>/8.0f
					},
					.phase{
						.offset = 2.0f - std::numbers::phi_v<float>,
						.offset_noise = 1.0f/12.0f
					}
				},
				.wave_properties{
					.wavelength = 10240.0f,
					.phase = -0.5f*std::numbers::pi_v<float>
				}
			},
			.x_distortion{
				.shape{
					.amplitude{
						.scaling_factor = 2.0f*std::numbers::phi_v<float>,
						.scaling_noise = std::numbers::phi_v<float>/8.0f
					},
					.wavelength{
						.scaling_factor = std::numbers::phi_v<float>,
						.scaling_noise = std::numbers::phi_v<float>/8.0f
					},
					.phase{
						.offset = 2.0f - std::numbers::phi_v<float>,
						.offset_noise = 1.0f/12.0f
					}
				},
				.wave_properties{
					.wavelength = 8192.0f,
					.phase = 0.0f
				}
			},
			.y_distortion{
				.shape{
					.amplitude{
						.scaling_factor = 4.0f*std::numbers::phi_v<float>,
						.scaling_noise = std::numbers::phi_v<float>/8.0f
					},
					.wavelength{
						.scaling_factor = std::numbers::phi_v<float>,
						.scaling_noise = std::numbers::phi_v<float>/8.0f
					},
					.phase{
						.offset = 2.0f - std::numbers::phi_v<float>,
						.offset_noise = 1.0f/12.0f
					}
				},
				.wave_properties{
					.wavelength = 8192.0f,
					.phase = 0.0f
				}
			},
			.half_length = 24576.0f
		}
	};

	random_generator rng;
	default_thread_pool threads{16};

	auto const ridge_curve = generate(rng,
		heightmap_params.main_ridge.ridge_curve,
		4096.0f,  //xy_amp
		heightmap_params.main_ridge.ridge_curve_xz,
		512.0f,   //xz_amp
		polyline_location_params{
			.point_count = domain_width,
			.dx = pixel_size,
			.start_location = heightmap_params.main_ridge.start_location
		}
	);

	double_buffer<grayscale_image> uplift_zone{domain_width, domain_height};

	{
		puts("Generating uplift zone");
		puts("   Generating boundary values");
		basic_image<dirichlet_boundary_pixel<float>> uplift_zone_boundary{domain_width, domain_height};
		auto const radius_distortion = generate(rng,
			heightmap_params.uplift_zone.radius_distortion,
			output_range{.min = -512.0f, .max = 512.0f},
			polyline_displacement_params{
				.point_count = domain_width,
				.dx = pixel_size
			}
		);
		auto const z_avg = heightmap_params.main_ridge.start_location[2];
		std::uniform_real_distribution init_noise{0.0f, z_avg};
		for(uint32_t y = 0; y != uplift_zone_boundary.height(); ++y)
		{
			for(uint32_t x = 0; x != uplift_zone_boundary.width(); ++x)
			{
				location const current_loc{
					pixel_size*static_cast<float>(x),
					pixel_size*static_cast<float>(y),
					0.0f
				};

				auto const i = std::ranges::min_element(ridge_curve, [current_loc](auto a, auto b) {
					return distance_xy(current_loc, a) < distance_xy(current_loc, b);
				});

				// NOTE: This works because main_ridge is a function of x

				auto const radius = (current_loc[1] < ridge_curve[x][1]?
					 heightmap_params.uplift_zone.radius_north
					:heightmap_params.uplift_zone.radius_south) + radius_distortion[x][1];
				auto const curve_z = (*i)[2];
				auto const z_0 = curve_z*radius/z_avg;
				auto const d = distance_xy(*i, current_loc);
				auto const zero_line = d - z_0;
				uplift_zone_boundary(x, y) = dirichlet_boundary_pixel{
					.weight = zero_line > 0.0f? 1.0f : 0.0f,
					.value = 0.0f
				};
				uplift_zone.back()(x, y) = init_noise(rng);
			}
		}
		uplift_zone.swap();

		draw(uplift_zone_boundary.pixels(), ridge_curve, line_segment_draw_params{
			.value = dirichlet_boundary_pixel{.weight = 1.0f, .value = 1.0f},
			.blend_function = [](auto, auto new_val, auto){
				return new_val;
			},
			.intensity_modulator = [](float curve_intensity, auto brush_value) {
				brush_value.value *= curve_intensity;
				return brush_value;
			},
			.scale = pixel_size
		});

		puts("   Running laplace solver");
		solve_bvp(uplift_zone, terraformer::laplace_solver_params{
			.tolerance = 0.5f,
			.step_executor_factory = std::ref(threads),
			.boundary = std::cref(uplift_zone_boundary)
		});

		store(uplift_zone.front(), "uplift_zone.exr");
	}

	auto const u_ridge = ridge_curve[0][1]/(pixel_size*static_cast<float>(domain_height));
	basic_image<std::pair<float, float>> coord_mapping{domain_width, domain_height};
	{
		double_buffer<grayscale_image> u_solve{domain_width, domain_height};
		puts("Generating distance field");
		puts("   Generating boundary values");
		basic_image<dirichlet_boundary_pixel<float>> ridge_line{domain_width, domain_height};
		std::uniform_real_distribution init_noise{0.0f, 1.0f/4.0f};
		draw(ridge_line.pixels(), ridge_curve, line_segment_draw_params{
			.value = dirichlet_boundary_pixel{
				.weight = 1.0f,
				.value = u_ridge
			},
			.blend_function = [](auto, auto new_val, auto){
				return new_val;
			},
			.intensity_modulator = [](auto, auto brush_value) {
				return brush_value;
			},
			.scale = pixel_size
		});

		puts("   Initiating laplace solver");
		{
			auto& init = u_solve.back();
			for(uint32_t y = 0; y != domain_height; ++y)
			{
				for(uint32_t x = 0; x != domain_width; ++x)
				{
					auto const y_val = static_cast<float>(y)/static_cast<float>(domain_height);
					init(x, y) = y_val + init_noise(rng);
				}
			}
		}
		u_solve.swap();

		puts("   Running laplace solver");
		solve_bvp(u_solve, terraformer::laplace_solver_params{
			.tolerance = 1.0f/static_cast<float>(std::max(domain_width, domain_height)),
			.step_executor_factory = std::ref(threads),
			.boundary = [&ridge_line](uint32_t x, uint32_t y)
			{
				if(y == 0 || y == domain_height - 1)
				{
					return dirichlet_boundary_pixel{
						.weight = 1.0f,
						.value = static_cast<float>(y)/static_cast<float>(domain_height)
					};
				}
				return ridge_line(x, y);
			}
		});

		float minval = std::numeric_limits<float>::infinity();
		float maxval = -std::numeric_limits<float>::infinity();
		for(uint32_t y = 0; y != domain_height; ++y)
		{
			auto v = 0.0f;
			for(uint32_t x = 0; x != domain_width; ++x)
			{
				coord_mapping(x, y).first = u_solve.front()(x, y);
				coord_mapping(x, y).second = v;
				minval = std::min(v, minval);
				maxval = std::max(v, maxval);

				auto const gradvec = direction{grad(u_solve.front().pixels(), x, y, 1.0f, clamp_at_boundary{})};
				auto const gradvec_conj_x = gradvec[1];
				v += gradvec_conj_x;
			}
		}
		for(uint32_t y = 0; y != domain_height; ++y)
		{
			for(uint32_t x = 0; x != domain_width; ++x)
			{ coord_mapping(x, y).second = (coord_mapping(x, y).second - minval)/(maxval - minval); }
		}
	}

	basic_image<float> base_elevation{domain_width, domain_height};
	{
		puts("Generating base elevation");
		for(uint32_t y = 0; y != base_elevation.height(); ++y)
		{
			for(uint32_t x = 0; x != base_elevation.width(); ++x)
			{
				auto const u = coord_mapping(x, y).first;
				auto const du = u - u_ridge;
				auto const v = coord_mapping(x, y).second;

				auto const z_boundary = du < 0.0f?
					std::lerp(heightmap_params.corners.nw.elevation, heightmap_params.corners.ne.elevation, v):
					std::lerp(heightmap_params.corners.sw.elevation, heightmap_params.corners.se.elevation, v);

				auto const eta = du < 0.0f?
					u/u_ridge:
					1.0f - du/(1.0f - u_ridge);
				auto const t = smoothstep(2.0f*(eta - 0.5f));
				base_elevation(x, y) = t*heightmap_params.main_ridge.base_elevation
					+ z_boundary*(1.0f - t);
			}
		}
		store(base_elevation, "base_elevation.exr");
	}

	basic_image<float> output{domain_width, domain_height};
	{
		puts("Mixing bumps with base elevation");
		for(uint32_t y = 0; y != output.height(); ++y)
		{
			for(uint32_t x = 0; x != output.width(); ++x)
			{
				auto const z_valley = base_elevation(x, y);
				auto const u = coord_mapping(x, y).first - u_ridge;
				auto const v = coord_mapping(x, y).second;
				auto const factor = std::exp2(-std::abs(u));
				auto const hills_value = approx_sine(2.0f*std::numbers::pi_v<float>*(3.0f*u/factor + 0.25f))
				 *0.5f*(1.0f + approx_sine(2.0f*std::numbers::pi_v<float>*(5*v)));
				auto const hills_amplitude = 1024.0f*factor;
				auto const z_hills = hills_amplitude*hills_value;

				auto const z_uplift = uplift_zone.front()(x, y);
				output(x, y) = z_valley + z_hills + z_uplift;
			}
		}
		store(output, "output.exr");
	}
}
