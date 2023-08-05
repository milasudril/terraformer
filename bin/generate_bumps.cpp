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

	basic_image<float> output{1024, 1024};
	random_generator rng;

	fractal_wave_params params_x
	{
		.wavelength = 16384.0f/48.0f,
		.scaling_factor = std::numbers::phi_v<float>,
		.scaling_noise = std::numbers::phi_v<float>/8.0f,
		.phase_shift = 2.0f - std::numbers::phi_v<float>,
		.phase_shift_noise = 1.0f/12.0f
	};

	fractal_wave_params params_y
	{
		.wavelength = 8192.0f/48.0f,
		.scaling_factor = 1.0f + std::numbers::phi_v<float>,
		.scaling_noise = std::numbers::phi_v<float>/16.0f,
		.phase_shift = 2.0f - std::numbers::phi_v<float>,
		.phase_shift_noise = 1.0f/6.0f
	};

	std::array<std::array<wave_component, 16>, 32> wave_components{};
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
	scaling const lambda{params_x.wavelength, params_y.wavelength, 1.0f};
	static constexpr auto pi = std::numbers::pi_v<float>;
	for(size_t k = 0; k != std::size(wave_components); ++k)
	{
		for(size_t l = 0; l != std::size(wave_components[k]); ++l)
		{
			auto const r = displacement{
				static_cast<float>(l) - 0.5f*static_cast<float>(std::size(wave_components[k])),
				static_cast<float>(k),
				0.0f
			} + displacement{0.5f, 0.5f, 0.0f};
			auto const k_hat = direction{r};
			auto const scaling_factor = std::exp2(-norm((r + displacement{sn_x(rng), sn_y(rng), 0.0f}).apply(decay_rates)));
			wave_components[k][l] = wave_component{
				.amplitude = scaling_factor,
				.phase = pi*norm(displacement{r}.apply(phase_shift) + displacement{psn_x(rng), psn_y(rng), 0.0f}),
				.wave_vector = (2.0f*pi*k_hat/scaling_factor).apply(inverted(lambda))
			};
		}
	}



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
				for(size_t l = 0; l != std::size(wave_components[k]); ++l)
				{
					auto const A = wave_components[k][l].amplitude;
					auto const phase = wave_components[k][l].phase;
					auto const wave_vector = wave_components[k][l].wave_vector;
					sum += A*std::cos(inner_product(v, wave_vector) + phase);
				}
			}
			output(x, y) = sum;
		}
	}

	store(output, "bumps.exr");
}
