#ifndef TERRAFORMER_DAMPED_MOTION_INTEGRATOR_HPP
#define TERRAFORMER_DAMPED_MOTION_INTEGRATOR_HPP

#include "./spaces.hpp"

#include <random>

namespace terraformer
{
	struct noisy_drift_params
	{
		geosimd::rotation_angle drift;
		float noise_amount;
	};

	template<class Rng>
	class noisy_drift
	{
	public:
		using params = noisy_drift_params;

		constexpr explicit noisy_drift(Rng rng, params model_params):
			m_rng{rng},
			m_model_params{model_params}
		{ }

		displacement operator()()
		{
			std::uniform_real_distribution U{-1.0f, 1.0f};
			while(true)
			{
				displacement const v{U(m_rng), U(m_rng), 0.0f};
				if(auto const norm_random_sq = norm_squared(v); norm_random_sq < 1.0f) [[likely]]
				{
					auto const theta_drift = m_model_params.drift
						- geosimd::rotation_angle{geosimd::turns{0.0}};
					auto const theta_random = angular_difference(direction{v}, direction{geom_space::x{}});
					auto const norm_random = std::sqrt(norm_random_sq);
					auto const r = (1.0f - m_model_params.noise_amount)
						+ 1.5f*norm_random*m_model_params.noise_amount;
					auto const theta = (1.0f - m_model_params.noise_amount)*theta_drift
						+ m_model_params.noise_amount*theta_random;
					auto const cs = cossin(theta);

					return r*displacement{cs.cos, cs.sin, 0.0f};
				}
			}
		}

	private:
		Rng m_rng;
		params m_model_params;
	};
}

#endif