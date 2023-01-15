#ifndef TERRAFORMER_LIB_TURN_ANGLE_LIMITER_HPP
#define TERRAFORMER_LIB_TURN_ANGLE_LIMITER_HPP

#include "./particle_state.hpp"

#include <geosimd/angle.hpp>

namespace terraformer
{
	class turn_angle_limiter
	{
	public:
		struct params
		{
			geosimd::turn_angle max_turn_angle;
			float scaling_factor;
		};

		struct state
		{
			struct particle_state p_state;
			direction dir;
			geosimd::turn_angle integrated_heading;
		};

		constexpr explicit turn_angle_limiter(state const& initial_state, params model_params):
			m_state_prev{initial_state},
			m_model_params{model_params}
		{ }

		particle_state operator()(particle_state const& new_state)
		{
			auto const state_in = m_state_prev;

			auto const dr = new_state.r - state_in.p_state.r;
			auto const dir = direction{dr};
			auto const dir_prev = state_in.dir;
			auto const dtheta_in = angular_difference(dir, dir_prev);
			auto const max_turn_angle = m_model_params.max_turn_angle;

			auto dtheta = clamp(dtheta_in, -max_turn_angle, max_turn_angle);

			// This prevents the curve from turning backwards. This is a sufficient condition to
			// prevent self-intersections
			geosimd::turn_angle const max_heading{0x4000'0000};
			auto const iheading_prev = state_in.integrated_heading;
			if(dtheta + iheading_prev > max_heading)
			{ dtheta = max_heading - iheading_prev; }
			else
			if(dtheta - iheading_prev < -max_heading)
			{ dtheta = -max_heading - iheading_prev; }

			auto const theta_ref = angular_difference(dir_prev, direction{geom_space::x{}});
			auto const cs = cossin(theta_ref + dtheta);
			auto const dir_corr = direction{cs, geom_space::z{}}; // As if rotated ccw around z-axis
			// Use the magnitude of the projection of dr on to the corrected direction. This
			// simulates a collision that preserves the tangential velocity is kept, while resets
			// the normal velocity
			auto const d = std::abs(inner_product(dir_corr, dr));

			auto const r = state_in.p_state.r + d*dir_corr;
			// Remember to update the velocity to match the new location
			auto const v = 2.0f*(r - state_in.p_state.r)/m_model_params.scaling_factor
				- state_in.p_state.v;

			m_state_prev = state{
				.p_state = new_state,
				.dir = dir,
				.integrated_heading = iheading_prev + dtheta
			};

			return particle_state{v, r};
		}

	private:
		state m_state_prev;
		params m_model_params;
	};
}

#endif