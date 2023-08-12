//@	{"target":{"name":"generate_bumps.o"}}

#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/image_io.hpp"
#include "lib/common/spaces.hpp"
#include "lib/curve_tool/ridge_curve.hpp"

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

struct bump_field
{
	terraformer::fractal_wave::params wave_params;
	float wavelength;
	float amplitude;
};

struct steady_plate_collision_zone_descriptor
{
	struct corners corners;
	terraformer::main_ridge_params main_ridge;
	struct bump_field bump_field;
};

int main()
{
	using namespace terraformer;

	static constexpr auto pixel_size = 48.0f;

	steady_plate_collision_zone_descriptor const heightmap_params{
		.corners{
			.sw = corner{512.0f},
			.se = corner{1536.0f},
			.nw = corner{2560.0f},
			.ne = corner{3584.0f},
		},
		.main_ridge{
			.start_location = terraformer::location{0.0f, 16384.0f, 0.0f},
			.distance_to_endpoint = 49152.0f,
			.ridge_line{
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
			.base_elevation = 5120.0f
		},
		.bump_field{
			.wave_params{
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
			.wavelength = 5120.0f,
			.amplitude = 1024.0f
		}
	};

	random_generator rng;
	auto const main_ridge = generate(rng, pixel_size, heightmap_params.main_ridge);

	basic_image<float> bump_field{1024, 1024};
	auto max_val = -16384.0f;
	auto min_val = 16384.0f;

	{
		puts("Generating bumps");
		terraformer::fractal_wave ridege_wave{rng, heightmap_params.bump_field.wave_params};
		for(uint32_t y = 0; y != bump_field.height(); ++y)
		{
			printf(" %3x\r",static_cast<int>(256*static_cast<float>(y)/static_cast<float>(bump_field.height())));
			fflush(stdout);
			for(uint32_t x = 0; x != bump_field.width(); ++x)
			{
				auto const xf = pixel_size*static_cast<float>(x);
				auto const yf = pixel_size*static_cast<float>(y);
				location const current_loc{xf, yf, 0.0f};

				auto convsum = 0.0f;
				for(size_t k = 0; k != std::size(main_ridge); ++k)
				{
					auto const d = distance(current_loc, main_ridge[k]);
					convsum += ridege_wave(d/heightmap_params.bump_field.wavelength);
				}
				min_val = std::min(convsum, min_val);
				max_val = std::max(convsum, max_val);
				bump_field(x, y) = convsum;
			}
		}

		for(uint32_t y = 0; y != bump_field.height(); ++y)
		{
			for(uint32_t x = 0; x != bump_field.width(); ++x)
			{
				auto const val = bump_field(x, y);
				bump_field(x, y) = (val - min_val)/(max_val - min_val);
			}
		}
		store(bump_field, "bumps.exr");
	}

	basic_image<float> base_elevation{1024, 1024};
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
				auto const side = current_loc[1] < main_ridge[x][1]? -1.0f : 1.0f;

				auto const i = std::ranges::min_element(main_ridge, [current_loc](auto a, auto b) {
					return distance(current_loc, a) < distance(current_loc, b);
				});
				auto const ridge_point = *i;

				auto const distance_to_ridge = distance(current_loc, ridge_point);
				auto const xi = static_cast<float>(x)/(w - 1.0f);
				auto const z_boundary = side < 0.0f?
					std::lerp(heightmap_params.corners.nw.elevation, heightmap_params.corners.ne.elevation, xi):
					std::lerp(heightmap_params.corners.sw.elevation, heightmap_params.corners.se.elevation, xi);
				auto const distance_to_boundary = side < 0.0f?
					current_loc[1]:
					pixel_size*h - current_loc[1];
				auto const eta = distance_to_boundary/(distance_to_boundary + distance_to_ridge);
				auto const z_valley = z_boundary + eta*eta*(heightmap_params.main_ridge.base_elevation - z_boundary);
				base_elevation(x, y) = z_valley;
			}
		}
		store(base_elevation, "base_elevation.exr");
	}

	basic_image<float> output{1024, 1024};
	{
		puts("Mixing bumps with base elevation");

		for(uint32_t y = 0; y != output.height(); ++y)
		{
			for(uint32_t x = 0; x != output.width(); ++x)
			{
				auto const z_valley = base_elevation(x, y);
				auto const wave_val = bump_field(x, y);
				auto const val = wave_val != 1.0f ? 1.0f - (1.0f - wave_val)/std::sqrt(1.0f - wave_val) : 1.0f;
				output(x, y) = z_valley*(1.0f + heightmap_params.bump_field.amplitude*2.0f*(val - 0.5f)/heightmap_params.main_ridge.base_elevation);
			}
		}
		store(output, "output.exr");
	}
}
