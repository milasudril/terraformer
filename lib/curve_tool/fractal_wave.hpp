//@	{"dependencies_extra":[{"ref": "./fractal_wave.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_FRACTALWAVE_HPP
#define TERRAFORMER_FRACTALWAVE_HPP

#include "./polyline.hpp"
#include "lib/common/utils.hpp"
#include "lib/common/output_range.hpp"

#include <numbers>
#include <random>
#include <cmath>
#include <array>

namespace terraformer
{
	struct exponential_progression
	{
		float scaling_factor;
		float scaling_noise;
	};

	template<class Rng>
	inline float get_value(exponential_progression const& val, size_t k, Rng&& rng)
	{
		std::uniform_real_distribution U{-0.5f, 0.5f};
		return std::pow(val.scaling_factor, -(static_cast<float>(k) + val.scaling_noise*U(rng)));
	}

	struct linear_progression
	{
		float offset;
		float offset_noise;
	};

	template<class Rng>
	inline float get_value(linear_progression const& val, size_t k, Rng&& rng)
	{
		std::uniform_real_distribution U{-0.5f, 0.5f};
		return -(static_cast<float>(k)*val.offset + val.offset_noise*U(rng));
	}

	struct wave_params
	{
		float wavelength;
		float phase;
	};

	struct wave_component
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
		explicit fractal_wave(Rng&& rng, params const& params)
		{
			std::uniform_real_distribution U{-0.5f, 0.5f};
			for(size_t k = 0; k != std::size(m_components); ++k)
			{
				m_components[k] = wave_component{
					.amplitude = get_value(params.amplitude, k, rng),
					.wavelength = get_value(params.wavelength, k, rng),
					.phase = get_value(params.phase, k, rng)
				};
			}
		}

		auto operator()(float x) const
		{
			auto sum = 0.0f;
			auto constexpr twopi = 2.0f*std::numbers::pi_v<float>;
			for(size_t k = std::size(m_components); k != 0; --k)
			{
				auto const amplitude = m_components[k - 1].amplitude;
				auto const wavelength = m_components[k - 1].wavelength;
				auto const phase = m_components[k - 1].phase;

				sum += amplitude*approx_sine(twopi*(x/wavelength - phase));
			}
			return sum;
		}

	private:
		std::array<wave_component, 16> m_components;
	};

	struct fractal_wave_params
	{
		fractal_wave::params shape;
		wave_params wave_properties;
	};

	std::vector<displacement> generate(fractal_wave const& wave,
		wave_params const& wave_params,
		output_range output_range,
		polyline_displacement_params const& line_params);

	std::vector<location> generate(fractal_wave const& wave,
		wave_params const& wave_params,
		output_range output_range,
		polyline_location_params const& line_params);

	template<class Rng, class ... Params>
	auto generate(Rng&& rng,
		fractal_wave_params const& wave_params,
		output_range output_range,
		Params&&... params)
	{
		return generate(fractal_wave{rng, wave_params.shape},
			wave_params.wave_properties,
			output_range,
			std::forward<Params>(params)...);
	}

	std::vector<location> generate(fractal_wave const& wave_xy,
		wave_params const& wave_xy_params,
		float xy_amp,
		fractal_wave const& wave_xz,
		wave_params const& wave_xz_params,
		float xz_amp,
		polyline_location_params const& line_params);

	template<class Rng, class ... Params>
	auto generate(Rng&& rng,
		fractal_wave_params const& wave_params_xy,
		float xy_amp,
		fractal_wave_params const& wave_params_xz,
		float xz_amp,
		Params&&... params)
	{
		return generate(fractal_wave{rng, wave_params_xy.shape},
			wave_params_xy.wave_properties,
			xy_amp,
			fractal_wave{rng, wave_params_xz.shape},
			wave_params_xz.wave_properties,
			xz_amp,
			std::forward<Params>(params)...);
	}
}

#endif