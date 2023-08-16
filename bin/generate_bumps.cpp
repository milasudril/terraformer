//@	{"target":{"name":"generate_bumps.o"}}

#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/image_io.hpp"
#include "lib/common/spaces.hpp"
#include "lib/filters/diffuser.hpp"
#include "lib/filters/curve_rasterizer.hpp"
#include "lib/curve_tool/fractal_wave.hpp"
#include "lib/execution/thread_pool.hpp"
#include "lib/filters/diffuser.hpp"

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
	terraformer::fractal_wave_params ridge_curve_xy;
	terraformer::fractal_wave_params ridge_curve_xz;
};

struct uplift_zone
{
	float radius;
};

struct bump_field
{
	terraformer::fractal_wave_params impact_waves;
	terraformer::fractal_wave_params x_distortion;
	terraformer::fractal_wave_params y_distortion;
};

struct steady_plate_collision_zone_descriptor
{
	struct corners corners;
	main_ridge_params main_ridge;
	struct uplift_zone uplift_zone;
	struct bump_field bump_field;
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
			.ne = corner{4068.0f},
		},
		.main_ridge{
			.start_location = terraformer::location{0.0f, 16384.0f, 3072.0f},
			.distance_xy_to_endpoint = 49152.0f,
			.base_elevation = 5120.0f,
			.ridge_curve_xy{
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
					.amplitude = 4096.0f,
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
					.amplitude = 512.0f,
					.wavelength = 8192.0f,
					.phase = 0.0f
				}
			}
		},
		.uplift_zone{
			.radius = 12384.0f
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
					.amplitude = 2048.0f,
					.wavelength = 8192.0f,
					.phase = 0.0f
				}
			},
			.x_distortion{
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
					.amplitude = 768.0f,
					.wavelength = 13312.0f,
					.phase = 0.0f
				}
			},
			.y_distortion{
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
					.amplitude = 64.0f,
					.wavelength = 5120.0f,
					.phase = 0.0f
				}
			}
		}
	};

	random_generator rng;
	default_thread_pool threads{16};

	auto const ridge_curve_xy = generate(rng,
		heightmap_params.main_ridge.ridge_curve_xy,
		heightmap_params.main_ridge.ridge_curve_xz,
		uniform_polyline_params{
			.start_location = heightmap_params.main_ridge.start_location,
			.point_count = domain_width,
			.dx = pixel_size
		}
	);

	basic_image<float> bump_field{domain_width, domain_height};
	{
		puts("Generating bumps");
		auto max_val = -16384.0f;
		auto min_val = 16384.0f;
		terraformer::fractal_wave const ridege_wave{rng, heightmap_params.bump_field.impact_waves.shape};
		terraformer::fractal_wave const x_distortion{rng, heightmap_params.bump_field.x_distortion.shape};
		terraformer::fractal_wave const y_distortion{rng, heightmap_params.bump_field.y_distortion.shape};
		auto const now = std::chrono::steady_clock::now();
		for(uint32_t y = 0; y != bump_field.height(); ++y)
		{
			for(uint32_t x = 0; x != bump_field.width(); ++x)
			{
				auto const xf = pixel_size*static_cast<float>(x);
				auto const yf = pixel_size*static_cast<float>(y);
				auto const current_loc = location{xf, yf, 0.0f}
					+ displacement{
						x_distortion(yf/heightmap_params.bump_field.x_distortion.wave_properties.wavelength
							+ heightmap_params.bump_field.x_distortion.wave_properties.phase),
						y_distortion(xf/heightmap_params.bump_field.y_distortion.wave_properties.wavelength
							+ heightmap_params.bump_field.y_distortion.wave_properties.phase),
						0.0f
					}.apply(scaling{
						heightmap_params.bump_field.x_distortion.wave_properties.amplitude,
						heightmap_params.bump_field.y_distortion.wave_properties.amplitude,
						1.0f
					});

				auto convsum = 0.0f;
				for(size_t k = 0; k != std::size(ridge_curve_xy); ++k)
				{
					auto const d = distance_xy(current_loc, ridge_curve_xy[k]);
					convsum += ridege_wave(d/heightmap_params.bump_field.impact_waves.wave_properties.wavelength
						+ heightmap_params.bump_field.impact_waves.wave_properties.phase);
				}
				min_val = std::min(convsum, min_val);
				max_val = std::max(convsum, max_val);
				bump_field(x, y) = convsum;
			}
		}
		auto const t_end = std::chrono::steady_clock::now();
		printf("%.8g\n", std::chrono::duration<double>{t_end - now}.count());

		for(uint32_t y = 0; y != bump_field.height(); ++y)
		{
			for(uint32_t x = 0; x != bump_field.width(); ++x)
			{
				auto const gen_val = bump_field(x, y);
				auto const val_normalized = (gen_val - min_val)/(max_val - min_val);
				auto const val = val_normalized != 1.0f?
					1.0f - (1.0f - val_normalized)/std::sqrt(1.0f - val_normalized) :
					1.0f;
				bump_field(x, y) = heightmap_params.bump_field.impact_waves.wave_properties.amplitude*2.0f*(val - 0.5f);
			}
		}
		store(bump_field, "bumps.exr");
	}

	double_buffer<grayscale_image> uplift_zone{domain_width, domain_height};

	{
		puts("Generating uplift zone");
		puts("   Generating boundary values");
		basic_image<dirichlet_boundary_pixel<float>> uplift_zone_boundary{domain_width, domain_height};
		draw(uplift_zone_boundary.pixels(), ridge_curve_xy, line_segment_draw_params{
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
		for(uint32_t y = 0; y != uplift_zone_boundary.height(); ++y)
		{
			for(uint32_t x = 0; x != uplift_zone_boundary.width(); ++x)
			{
				location const current_loc{
					pixel_size*static_cast<float>(x),
					pixel_size*static_cast<float>(y),
					0.0f
				};

				auto const i = std::ranges::min_element(ridge_curve_xy, [current_loc](auto a, auto b) {
					return distance_xy(current_loc, a) < distance_xy(current_loc, b);
				});

				if(distance_xy(*i, current_loc) > heightmap_params.uplift_zone.radius)
				{ uplift_zone_boundary(x, y) = dirichlet_boundary_pixel{.weight = 1.0f, .value = 0.0f}; }
			}
		}

		puts("   Running laplace solver");
		solve_bvp(uplift_zone, terraformer::laplace_solver_params{
			.tolerance = 1.0e-6f * heightmap_params.main_ridge.start_location[2],
			.step_executor_factory = std::ref(threads),
			.boundary = std::cref(uplift_zone_boundary)
		});

		store(uplift_zone.front(), "uplift_zone.exr");
	}

	basic_image<float> base_elevation{domain_width, domain_height};
	{
		puts("Generating base elevation");
		auto const w = static_cast<float>(base_elevation.width());
		auto const h = static_cast<float>(base_elevation.height());

		for(uint32_t y = 0; y != base_elevation.height(); ++y)
		{
			for(uint32_t x = 0; x != base_elevation.width(); ++x)
			{
				location const current_loc{
					pixel_size*static_cast<float>(x),
					pixel_size*static_cast<float>(y),
					0.0f
				};

				// NOTE: This works because main_ridge is a function of x
				auto const side = current_loc[1] < ridge_curve_xy[x][1]? -1.0f : 1.0f;

				auto const i = std::ranges::min_element(ridge_curve_xy, [current_loc](auto a, auto b) {
					return distance_xy(current_loc, a) < distance_xy(current_loc, b);
				});
				auto const ridge_point = *i;

				auto const distance_xy_to_ridge = distance_xy(current_loc, ridge_point);
				auto const xi = static_cast<float>(x)/(w - 1.0f);
				auto const z_boundary = side < 0.0f?
					std::lerp(heightmap_params.corners.nw.elevation, heightmap_params.corners.ne.elevation, xi):
					std::lerp(heightmap_params.corners.sw.elevation, heightmap_params.corners.se.elevation, xi);
				auto const distance_xy_to_boundary = side < 0.0f?
					current_loc[1]:
					pixel_size*h - current_loc[1];
				auto const eta = distance_xy_to_boundary/(distance_xy_to_boundary + distance_xy_to_ridge);
				auto const z_valley = z_boundary + eta*eta*(heightmap_params.main_ridge.base_elevation - z_boundary);
				base_elevation(x, y) = z_valley;
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
				auto const z_hills = bump_field(x, y);
				auto const z_uplift = uplift_zone.front()(x, y);
				output(x, y) = z_valley*(1.0f + z_hills/heightmap_params.main_ridge.base_elevation) + z_uplift;
			}
		}
		store(output, "output.exr");
	}
}
