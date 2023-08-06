//@	{"target":{"name":"generate_bumps.o"}}

#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/image_io.hpp"
#include "lib/common/spaces.hpp"

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


int main()
{
	using namespace terraformer;

	auto xy_mix = 0.25f;

	fractal_wave_params const params_x
	{
		.wavelength = 8192.0f/48.0f,
		.scaling_factor = std::numbers::phi_v<float>,
		.scaling_noise = 0.0,
		.phase_shift = 2.0f - std::numbers::phi_v<float>,
		.phase_shift_noise = 0.0f
	};

	fractal_wave_params const params_y
	{
		.wavelength = 5120.0f/48.0f,
		.scaling_factor = std::numbers::phi_v<float>,
		.scaling_noise = 0.0f,
		.phase_shift = 2.0f - std::numbers::phi_v<float>,
		.phase_shift_noise = 0.0f
	};

	std::array<wave_component, 17*33> wave_components{};
	{
		random_generator rng;
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

	{
		for(uint32_t y = 0; y != output.height(); ++y)
		{
			for(uint32_t x = 0; x != output.width(); ++x)
			{
				auto const xf = static_cast<float>(x);
				auto const yf = static_cast<float>(y);
				displacement const v{xf, yf, 0.0f};
				auto sum = 0.0f;

				for(size_t k = 0; k != std::size(wave_components); ++k)
				{
						auto const A = wave_components[k].amplitude;
						auto const phase = wave_components[k].phase;
						auto const wave_vector = wave_components[k].wave_vector;
						sum += A*std::cos(inner_product(v, wave_vector) + phase);
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
	for(uint32_t y = 0; y != output.height(); ++y)
	{
		for(uint32_t x = 0; x != output.width(); ++x)
		{
			auto const h = static_cast<float>(output.height());
			auto const eta = 1.0f - 2.0f*std::abs(0.5f - static_cast<float>(y)/h);
			output_1(x, y) = (2048.0f + 3072.0f*eta*eta)*(1.0f + 1024*output(x, y)/5120.0f);
		}
	}


	store(output_1, "bumps.exr");
}
