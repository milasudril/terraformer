#ifndef TERRAFORMER_LIB_RIDGE_CURVE_HPP
#define TERRAFORMER_LIB_RIDGE_CURVE_HPP

#include "./fractal_wave.hpp"
#include "lib/common/spaces.hpp"
#include <vector>

namespace terraformer
{
	struct main_ridge_params
	{
		location start_location;
		float distance_to_endpoint;
		fractal_wave_params ridge_line;
		float base_elevation;
	};

	template<class Rng>
	std::vector<location> generate(Rng&& rng, float dx, main_ridge_params const& params)
	{
		auto const n_points = static_cast<size_t>(params.distance_to_endpoint/dx);
		std::uniform_real_distribution U{-0.5f, 0.5f};

		auto const wavelength = params.ridge_line.wave_properties.wavelength;
		auto const phase = U(rng) + params.ridge_line.wave_properties.phase;

		fractal_wave wave_xy{
			rng,
			params.ridge_line.shape
		};

		std::vector<displacement> curve;
		float wave_amplitude = 0.0f;
		for(size_t k = 0; k != n_points; ++k)
		{
			auto const x = static_cast<float>(k)*dx;
			auto const y = wave_xy(x/wavelength + phase);
			curve.push_back(displacement{x, y, 0.0f});
			wave_amplitude = std::max(std::abs(wave_amplitude), y);
		}

		scaling const scaling{1.0f, params.ridge_line.wave_properties.amplitude/wave_amplitude, 1.0f};

		std::vector<location> ret;
		for(size_t k = 0; k != std::size(curve); ++k)
		{ ret.push_back(params.start_location + curve[k].apply(scaling)); }
		return ret;
	}
}
#endif