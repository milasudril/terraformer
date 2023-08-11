#ifndef TERRAFORMER_LIB_FRACTALWAVE_HPP
#define TERRAFORMER_LIB_FRACTALWAVE_HPP

#include <numbers>
#include <random>
#include <cmath>
#include <array>

namespace terraformer
{
	struct exponential_progression
	{
		float initial_value;
		float scaling_factor;
		float scaling_noise;
	};

	template<class Rng>
	inline float get_value(exponential_progression const& val, size_t k, Rng&& rng)
	{
		std::uniform_real_distribution U{-0.5f, 0.5f};
		return val.initial_value*std::pow(val.scaling_factor, -(static_cast<float>(k) + val.scaling_noise*U(rng)));
	}

	struct linear_progression
	{
		float initial_value;
		float offset;
		float offset_noise;
	};

	template<class Rng>
	inline float get_value(linear_progression const& val, size_t k, Rng&& rng)
	{
		std::uniform_real_distribution U{-0.5f, 0.5f};
		return val.initial_value - (static_cast<float>(k)*val.offset + val.offset_noise*U(rng));
	}

	struct wave_params
	{
		float amplitude;
		float wavelength;
		float phase;
	};

	class fractal_wave
	{
	public:
		struct params
		{
			exponential_progression amplitude;
			exponential_progression wavelength;
			linear_progression phase;
		};

		template<class Rng>
		explicit fractal_wave(Rng&& rng, params const& params):m_amplitude{0.0f}
		{
			std::uniform_real_distribution U{-0.5f, 0.5f};
			for(size_t k = 0; k != std::size(m_component_params); ++k)
			{
				m_component_params[k] = wave_params{
					.amplitude = get_value(params.amplitude, k, rng),
					.wavelength = get_value(params.wavelength, k, rng),
					.phase = get_value(params.phase, k, rng)
				};
			}
		}

		auto operator()(float x)
		{
			auto sum = 0.0f;
			auto constexpr twopi = 2.0f*std::numbers::pi_v<float>;
			for(size_t k = std::size(m_component_params); k != 0; --k)
			{
				auto const amplitude = m_component_params[k - 1].amplitude;
				auto const wavelength = m_component_params[k - 1].wavelength;
				auto const phase = m_component_params[k - 1].phase;

				sum += amplitude*std::sin(twopi*(x/wavelength - phase));
			}

			m_amplitude = std::max(std::abs(sum), m_amplitude);
			return sum;
		}

		float amplitude() const
		{ return m_amplitude; }

	private:
		std::array<wave_params, 16> m_component_params;
		float m_amplitude;
	};
}

#endif