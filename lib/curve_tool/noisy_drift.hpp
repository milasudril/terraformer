#ifndef TERRAFORMER_LIB_NOISY_DRIFT_HPP
#define TERRAFORMER_LIB_NOISY_DRIFT_HPP

#include "lib/common/spaces.hpp"

#include <random>

namespace terraformer
{
	class noisy_drift
	{
	public:
		struct params
		{
			geosimd::rotation_angle drift;
			float noise_amount;
		};

		constexpr explicit noisy_drift(params model_params):
			m_model_params{model_params}
		{ }

		template<class Rng>
		displacement operator()(Rng&& rng) const
		{
			std::uniform_real_distribution U{-1.0f, 1.0f};
			while(true)
			{
				displacement const v{U(rng), U(rng), 0.0f};
				if(auto const norm_random_sq = norm_squared(v); norm_random_sq < 1.0f) [[likely]]
				{
					auto const theta_drift = m_model_params.drift
						- geosimd::rotation_angle{geosimd::turns{0.0}};
					auto const theta_random = angular_difference(direction{v}, direction{geom_space::x{}});
					auto const norm_random = std::sqrt(norm_random_sq);
					auto const r = (1.0f - m_model_params.noise_amount)
						+ 1.5f*norm_random*m_model_params.noise_amount;
					auto const theta = theta_drift + m_model_params.noise_amount*theta_random;
					auto const cs = cossin(theta);

					return r*displacement{cs.cos, cs.sin, 0.0f};
				}
			}
		}

	private:
		params m_model_params;
	};
}

#endif