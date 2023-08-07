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

	auto xy_mix = 0.25f;
	static constexpr auto pixel_size = 48.0f;

	fractal_wave_params const params_x
	{
		.wavelength = 8192.0f/pixel_size,
		.scaling_factor = std::numbers::phi_v<float>,
		.scaling_noise = std::numbers::phi_v<float>/8.0f,
		.phase_shift = 2.0f - std::numbers::phi_v<float>,
		.phase_shift_noise = 1.0f/12.0f
	};

	fractal_wave_params const params_y
	{
		.wavelength = 5120.0f/pixel_size,
		.scaling_factor = std::numbers::phi_v<float>,
		.scaling_noise = std::numbers::phi_v<float>/2.0f,
		.phase_shift = 2.0f - std::numbers::phi_v<float>,
		.phase_shift_noise = 1.0f/24.0f
	};

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

	fractal_wave::params const horz_shift_params{
		.wavelength = 8192.0f,
		.per_wave_component_scaling_factor = std::numbers::phi_v<float>,
		.exponent_noise_amount = std::numbers::phi_v<float>/16.0f,
		.per_wave_component_phase_shift = 2.0f - std::numbers::phi_v<float>,
		.phase_shift_noise_amount = 1.0f/12.0f
	};

	random_generator rng;
	std::array<wave_component, 17*33> wave_components{};
	{
		scaling const decay_rates{
			std::log2(params_x.scaling_factor),
			std::log2(params_y.scaling_factor),
			0.0f
		};
		scaling const phase_shift{
			params_x.phase_shift,
			params_y.phase_shift,
			0.0f
		};
		std::uniform_real_distribution sn_x{-params_x.scaling_noise, params_x.scaling_noise};
		std::uniform_real_distribution sn_y{-params_y.scaling_noise, params_y.scaling_noise};
		std::uniform_real_distribution psn_x{-params_x.phase_shift_noise, params_x.phase_shift_noise};
		std::uniform_real_distribution psn_y{-params_y.phase_shift_noise, params_y.phase_shift_noise};
		auto const lambda = scaling{params_x.wavelength, params_y.wavelength, 1.0f}*
			scaling{params_x.scaling_factor, params_y.scaling_factor, 1.0f};
		auto const lambda_min = std::min(lambda[0], lambda[1]);
		displacement const A{1.0f - xy_mix, xy_mix, 1.0f};
		static constexpr auto pi = std::numbers::pi_v<float>;
		size_t index = 0;
		for(size_t k = 0; k != 17; ++k)
		{
			for(size_t l = 0; l != 33; ++l)
			{
				auto const r = displacement{
					static_cast<float>(l) - 16.0f,
					static_cast<float>(k),
					0.0f
				};
				auto const is_dc = (k == 0 && l == 16);
				auto const k_hat = direction{is_dc? displacement{1.0f, 0.0f, 0.0f} : r};
				auto const k_hat1 = is_dc? displacement{1.0f, 0.0f, 0.0f} : r/(std::abs(r[0]) + std::abs(r[1]));
				auto const scaling_factor = std::exp2(-norm(displacement{r}.apply(decay_rates)
					+ displacement{sn_x(rng), sn_y(rng), 0.0f}));
				wave_components[index] = wave_component{
					.amplitude = is_dc?
						0.0f : (scaling_factor*lambda_min < 2.0f ?
							0.0f :  std::abs(inner_product(k_hat1, A))*scaling_factor),
					.phase = pi*norm(displacement{r}.apply(phase_shift) + displacement{psn_x(rng), psn_y(rng), 0.0f}),
					.wave_vector = (2.0f*pi*k_hat/scaling_factor).apply(inverted(lambda))
				};
				++index;
			}
		}
		std::ranges::sort(wave_components, [](auto const& a, auto const& b) {
			return a.amplitude < b.amplitude;
		});
	}

	basic_image<float> output{1024, 1024};
	auto amplitude = 0.0f;
	auto const main_ridge = generate(rng, pixel_size, heightmap_params.main_ridge);
	terraformer::fractal_wave horz_shift{rng, 0.0f, horz_shift_params};
	auto const phi_0 = 0.5f*std::numbers::pi_v<float>;
	{
		for(uint32_t y = 0; y != output.height(); ++y)
		{
			for(uint32_t x = 0; x != output.width(); ++x)
			{
				auto const xf = static_cast<float>(x);
				auto const yf = static_cast<float>(y);
				location const current_loc{xf, yf, 0.0f};
				auto const i = std::ranges::min_element(main_ridge, [current_loc](auto a, auto b) {
					return distance(current_loc, a) < distance(current_loc, b);
				});

				auto sum = 0.0f;
				auto const v = current_loc - (*i) + 512.0f*displacement{horz_shift(yf*pixel_size), 0.0f, 0.0f}/pixel_size;

				for(size_t k = 0; k != std::size(wave_components); ++k)
				{
					auto const A = wave_components[k].amplitude;
					auto const phase = wave_components[k].phase;
					auto const wave_vector = wave_components[k].wave_vector;
					sum += A*std::cos(inner_product(v, wave_vector) + phase + phi_0);
				}
				amplitude = std::max(std::abs(sum), amplitude);
				output(x, y) = sum;
			}
		}
	}

	{
		for(uint32_t y = 0; y != output.height(); ++y)
		{
			for(uint32_t x = 0; x != output.width(); ++x)
			{ output(x, y) /= amplitude; }
		}
	}

	basic_image<float> output_1{1024, 1024};
	{
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

				output_1(x, y) = z_valley*(1.0f + 1024*output(x, y)/5120.0f);
			}
		}
	}

	store(output_1, "bumps.exr");
}
