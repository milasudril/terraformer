#ifndef TERRAFORMER_DAMPED_MOTION_INTEGRATOR_HPP
#define TERRAFORMER_DAMPED_MOTION_INTEGRATOR_HPP

#include "./spaces.hpp"

#include <random>

namespace terraformer
{
	struct noisy_drift_params
	{
		direction drift;
		float noise_amount;
	};

	template<class Rng>
	class noisy_drift
	{
	public:
		using params = noisy_drift_params;

		constexpr explicit noisy_drift(Rng rng, params const& model_params):
			m_rng{rng},
			m_model_params{model_params}
		{ }

		displacement operator()()
		{
			std::uniform_real_distribution U{-1.0f, 1.0f};
			while(true)
			{
				displacement const v{U(m_rng), U(m_rng), 0.0f};
				if(norm_squared(v) < 1.0f)
				{
					return (1.0f - m_model_params.noise_amount)*m_model_params.drift
						+ m_model_params.noise_amount*v;
				}
			}
		}

	private:
		Rng m_rng;
		params m_model_params;
	};
}

#endif