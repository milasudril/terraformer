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
	terraformer::basic_image<float> output{1024, 1024};
	random_generator rng;

	fractal_wave_params params_x
	{
		.wavelength = 1024.0f,
		.scaling_factor = std::numbers::phi_v<float>,
		.scaling_noise = std::numbers::phi_v<float>/16.0f,
		.phase_shift = 2.0f - std::numbers::phi_v<float>,
		.phase_shift_noise = 1.0f
	};

	fractal_wave_params params_y
	{
		.wavelength = 1024.0f,
		.scaling_factor = std::numbers::phi_v<float>,
		.scaling_noise = std::numbers::phi_v<float>/16.0f,
		.phase_shift = 2.0f - std::numbers::phi_v<float>,
		.phase_shift_noise = 1.0f
	};

	std::array<std::array<wave_component, 16>, 32> wave_components{};
	terraformer::displacement const rho{
		std::log2(params_x.scaling_factor),
		std::log2(params_y.scaling_factor),
		0.0f
	};
	auto const rho2 = rho.get()*rho.get();
	auto scaling_noise_x = std::uniform_real_distribution{-params_x.scaling_noise, params_x.scaling_noise};
	auto scaling_noise_y = std::uniform_real_distribution{-params_y.scaling_noise, params_y.scaling_noise};
	for(size_t k = 0; k != std::size(wave_components); ++k)
	{
		for(size_t l = 0; l != std::size(wave_components[k]); ++l)
		{
			auto const r =
			terraformer::displacement{
				static_cast<float>(l) - 0.5f*static_cast<float>(std::size(wave_components[k])),
				static_cast<float>(k),
				0.0f
			} + terraformer::displacement{0.5f, 0.5f, 0.0f};
			auto const noisy_r = r + terraformer::displacement{scaling_noise_x(rng), scaling_noise_y(rng), 0.0f};
			auto const r2 = noisy_r.get()*noisy_r.get();
			auto const k_hat = terraformer::direction{r};
			auto const scaling_factor = std::exp2(-std::sqrt(inner_product(r2, rho2)));

			wave_components[k][l] = wave_component{
				.amplitude = scaling_factor,
				.phase = norm(r)*params_x.phase_shift,
				.wave_vector = 2.0f*std::numbers::pi_v<float>*k_hat/(params_x.wavelength*scaling_factor)
			};
		}
	}



	for(uint32_t y = 0; y != output.height(); ++y)
	{
		for(uint32_t x = 0; x != output.width(); ++x)
		{
			auto const xf = static_cast<float>(x);
			auto const yf = static_cast<float>(y);
			terraformer::displacement const v{xf, yf, 0.0f};
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