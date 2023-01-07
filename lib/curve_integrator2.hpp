#ifndef TERRAFORMER_CURVE_INTEGRATOR_2_HPP
#define TERRAFORMER_CURVE_INTEGRATOR_2_HPP

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
		constexpr explicit damped_motion_integrator(displacement initial_input.
			float inertia,
			float curve_scaling_factor):
			m_input_prev{initial_input},
			m_inertia{inertia},
			m_curve_scaling_factor{curve_scaling_factor}
		{}

		constexpr auto operator()(particle_state const& state_prev, displacement input)
		{
			auto const input_prev = m_input_prev;
			m_input_prev = input;
			auto const inertia = m_inertia;
			auto const scaling_factor = m_curve_scaling_factor;

			auto const v = ((2.0f*inertia - 1.0f)/(1.0f + 2.0f*inertia)) * state_prev.v
				+ (input + input_prev)/(1.0f + 2.0f*inertia);
			auto const r =  state_prev.r + scaling_factor*(v + state_prev.v)/2.0f;

			return particle_state{v, r};
		}

	private:
		displacement m_input_prev;
		float m_inertia;
		float m_curve_scaling_factor;
	};
}

#endif