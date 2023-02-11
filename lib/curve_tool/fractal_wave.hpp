#ifndef TERRAFORMER_LIB_FRACTALWAVE_HPP
#define TERRAFORMER_LIB_FRACTALWAVE_HPP

#include <numbers>
#include <random>
#include <cmath>

namespace terraformer
{
	class fractal_wave
	{
	public:
		struct params
		{
			float x_offset;
			float wavelength;
			float per_wave_component_scaling_factor;
			float exponent_noise_amount;
			float per_wave_component_phase_shift;
			float phase_shift_noise_amount;
		};

		template<class Rng>
		explicit fractal_wave(Rng&& rng, params const& params):
			m_wavelength{params.wavelength},
			m_amplitude{0.0f}
		{
			std::uniform_real_distribution U{-0.5f, 0.5f};
			m_offset = params.x_offset;
			for(size_t k = 0; k != std::size(m_component_params); ++k)
			{
				m_component_params[k] = per_wave_component_params{
					.scale = std::pow(params.per_wave_component_scaling_factor, static_cast<float>(k)
						+ params.exponent_noise_amount*U(rng)),
					.phase_shift = static_cast<float>(k)*(params.per_wave_component_phase_shift
						+ params.phase_shift_noise_amount*U(rng))
				};
			}
		}

		auto operator()(float x)
		{
			auto sum = 0.0f;
			auto const twopi = 2.0f*std::numbers::pi_v<float>;
			auto const xi = x/m_wavelength;
			auto const offset = m_offset;
			for(size_t k = std::size(m_component_params); k != 0; --k)
			{
				auto const scale = m_component_params[k - 1].scale;
				auto const phase_shift = m_component_params[k - 1].phase_shift;
				sum += std::sin(twopi*(scale*(xi - offset) - phase_shift))/scale;
			}

			m_amplitude = std::max(std::abs(sum), m_amplitude);
			return sum;
		}

		float amplitude() const
		{
			return m_amplitude;
		}

	private:
		float m_wavelength;
		float m_offset;
		struct per_wave_component_params
		{
			float scale;
			float phase_shift;
		};

		std::array<per_wave_component_params, 16> m_component_params;
		float m_amplitude;
	};
}

#endif