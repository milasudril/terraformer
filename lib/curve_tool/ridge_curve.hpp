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
		fractal_wave::params wave_params;
		float wave_amplitude;
		float height_modulation;
	};

	template<class Rng>
	std::vector<location> generate(Rng&& rng, float dx, main_ridge_params const& params)
	{
		auto const n_points = static_cast<size_t>(params.distance_to_endpoint);
		std::uniform_real_distribution U{-0.5f, 0.5f};
		auto const x_offset = U(rng);
		fractal_wave wave_xy{
			rng,
			x_offset,
			params.wave_params
		};

		fractal_wave wave_xz{
			rng,
			x_offset,
			params.wave_params
		};

		std::vector<displacement> curve;
		for(size_t k = 0; k != n_points; ++k)
		{
			auto const x = static_cast<float>(k)*dx;
			auto const y = wave_xy(x);
			auto const z = wave_xz(x);
			curve.push_back(displacement{x, y, z});
		}

		auto const Ay = params.wave_amplitude/wave_xy.amplitude();
		auto const Az = params.height_modulation/(wave_xz.amplitude() * wave_xz.amplitude());

		std::vector<location> ret;
		for(size_t k = 0; k != std::size(curve); ++k)
		{
			auto const vec_val = curve[k].get();
			ret.push_back(params.start_location
				+ displacement{vec_val*geosimd::vec_t{1.0f, Ay, Az*vec_val[2], 1.0f}});
		}
		return ret;
	}
}
#endif