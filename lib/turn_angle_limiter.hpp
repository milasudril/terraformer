#ifndef TERRAFORMER_LIB_TURN_ANGLE_LIMITER_HPP
#define TERRAFORMER_LIB_TURN_ANGLE_LIMITER_HPP

#include "./particle_state.hpp"

namespace terraformer
{
	class turn_angle_limiter
	{
	public:
		struct params
		{
		};

		constexpr explicit turn_angle_limiter(params model_params):
			m_model_params{model_params}
		{ }

		displacement operator()(particle_state const& new_state)
		{
			auto const ps_in = m_old_state;
			auto const dr = new_state.r - ps_in.r;

			auto const dtheta_in = angular_difference(direction{dr}, m_dir_prev);
			geosimd::turn_angle const max_turn_angle{0x4000'0000};
			auto dtheta = clamp(dtheta_in, -max_turn_angle, max_turn_angle);

			geosimd::turn_angle const max_heading{0x4000'0000};
			if(dtheta + iheading_prev > max_heading)
			{ dtheta = max_heading - iheading_prev; }
			else
			if(dtheta + iheading_prev < -max_heading)
			{ dtheta = -max_heading - iheading_prev; }

			auto const theta_ref = angular_difference(m_dir_prev, direction{geom_space::x{}});
			auto const cs = cossin(theta_ref + dtheta);

			auto const dir_corr = direction{cs, geom_space::z{}}; // As if rotated ccw around z-axis

			// Use the magnitude of the projection of dr on to the corrected direction. This
			// simulates a collision that preserves the tangential velocity is kept, while resets
			// the normal velocity
			auto const d = std::abs(inner_product(dir_corr, dr));

			auto const r = ps_in.r + d*dir_corr;
			auto const v = 2.0f*(r - ps_in.r)/scaling_factor  - ps_in.v;
			auto const integ_distance = idist_prev + d;
			auto const integ_heading = iheading_prev + dtheta;

			return particle{
				.r = r,
				.v = v,
				.integ_distance = integ_distance,
				.integ_heading = integ_heading
			};
		}

	private:
		params m_model_params;
	};
}

#endif