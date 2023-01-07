#ifndef TERRAFORMER_DAMPED_MOTION_INTEGRATOR_HPP
#define TERRAFORMER_DAMPED_MOTION_INTEGRATOR_HPP

#include "./spaces.hpp"

#include <stdexcept>

namespace terraformer
{
	struct particle_state
	{
		displacement v;
		location r;
	};

	class damped_motion_integrator
	{
	public:
		struct params
		{
			float inertia;
			float curve_scaling_factor;
		};

		static constexpr auto min_inertia()
		{ return 0.5f; }

		constexpr explicit damped_motion_integrator(displacement initial_input,
			params model_params):
			m_input_prev{initial_input},
			m_params{model_params}
		{
			if(m_params.inertia < min_inertia())
			{ throw std::runtime_error{"Inertia out-of-range"}; }
		}

		constexpr auto operator()(particle_state const& state_prev, displacement input)
		{
			auto const input_prev = m_input_prev;
			m_input_prev = input;
			auto const inertia = m_params.inertia;
			auto const scaling_factor = m_params.curve_scaling_factor;

			auto const v = ((2.0f*inertia - 1.0f)/(1.0f + 2.0f*inertia)) * state_prev.v
				+ (input + input_prev)/(1.0f + 2.0f*inertia);
			auto const r =  state_prev.r + scaling_factor*(v + state_prev.v)/2.0f;

			return particle_state{v, r};
		}

	private:
		displacement m_input_prev;
		params m_params;
	};
}

#endif