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


struct fractal_wave_params
{
	float wavelength;
	float scaling_factor;
	float scaling_noise;
	float phase_shift;
	float phase_shift_noise;
};

struct wave_component
{
	float amplitude;
	float phase;
	terraformer::displacement wave_vector;
};

struct domain_boundary_conditions
{
	float front_level;
	float back_level;
};

struct steady_plate_collision_zone_descriptor
{
	domain_boundary_conditions boundary;
	terraformer::main_ridge_params main_ridge;
};


int main()
{
	using namespace terraformer;

	static constexpr auto pixel_size = 48.0f;

	steady_plate_collision_zone_descriptor const heightmap_params{
		.boundary{
			.front_level = 1024.0f,
			.back_level = 3072.0f
		},
		.main_ridge{
			.start_location = terraformer::location{0.0f, 16384.0f, 0.0f},
			.distance_to_endpoint = 49152.0f,
			.wave_params{
				.wavelength = 24576.0f,
				.per_wave_component_scaling_factor = std::numbers::phi_v<float>,
				.exponent_noise_amount = std::numbers::phi_v<float>/16.0f,
				.per_wave_component_phase_shift = 2.0f - std::numbers::phi_v<float>,
				.phase_shift_noise_amount = 1.0f/12.0f
			},
			.wave_amplitude = 4096.0f,
			.height_modulation = 0.0f
		}
	};

	fractal_wave::params const ridge_wave_params{
		.wavelength = 5120.0f,
		.per_wave_component_scaling_factor = std::numbers::phi_v<float>,
		.exponent_noise_amount = std::numbers::phi_v<float>/8.0f,
		.per_wave_component_phase_shift = 2.0f - std::numbers::phi_v<float>,
		.phase_shift_noise_amount = 1.0f/12.0f
	};

	random_generator rng;

	basic_image<float> output{1024, 1024};
	auto max_val = -16384.0f;
	auto min_val = 16384.0f;
	auto const main_ridge = generate(rng, pixel_size, heightmap_params.main_ridge);
	terraformer::fractal_wave ridege_wave{rng, 0.0f, ridge_wave_params};

	{
		puts("Generating wave");
		for(uint32_t y = 0; y != output.height(); ++y)
		{
			printf("%u   \r",y);
			fflush(stdout);
			for(uint32_t x = 0; x != output.width(); ++x)
			{
				auto const xf = pixel_size*static_cast<float>(x);
				auto const yf = pixel_size*static_cast<float>(y);
				location const current_loc{xf, yf, 0.0f};

				auto convsum = 0.0f;
				for(size_t k = 0; k != std::size(main_ridge); ++k)
				{
					auto const d = distance(current_loc, main_ridge[k]);
					convsum += ridege_wave(d);
				}

				min_val = std::min(convsum, min_val);
				max_val = std::max(convsum, max_val);
				output(x, y) = convsum;
			}
		}
	}

	{
		puts("Normalizing");
		for(uint32_t y = 0; y != output.height(); ++y)
		{
			for(uint32_t x = 0; x != output.width(); ++x)
			{
				auto const val = output(x, y);
				output(x, y) = (val - min_val)/(max_val - min_val);
			}
		}
	}

	basic_image<float> output_1{1024, 1024};
	{
		puts("Mixing");
		auto const h = output_1.height();

		for(uint32_t y = 0; y != output.height(); ++y)
		{
			for(uint32_t x = 0; x != output.width(); ++x)
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
				auto const z_boundary = side < 0.0f?
				heightmap_params.boundary.back_level:
					heightmap_params.boundary.front_level;
					auto const distance_to_boundary = side < 0.0f?
					current_loc[1]:
				pixel_size*static_cast<float>(h) - current_loc[1];
				auto const eta = distance_to_boundary/(distance_to_boundary + distance_to_ridge);
				auto const z_valley = z_boundary + eta*eta*(5120.0f - z_boundary);

				auto const wave_val = output(x, y);
				auto const val = wave_val != 1.0f ? 1.0f - (1.0f - wave_val)/std::sqrt(1.0f - wave_val) : 1.0f;
				output_1(x, y) = z_valley*(1.0f + 1024.0f*2.0f*(val - 0.5f)/5120.0f);
			}
		}
	}

	store(output_1, "bumps.exr");
}
