#ifndef TERRAFORMER_LIB_RIDGE_CURVE_HPP
#define TERRAFORMER_LIB_RIDGE_CURVE_HPP

#include "./noisy_drift.hpp"
#include "./turn_angle_limiter.hpp"
#include "./damped_motion_integrator.hpp"
#include "./particle_state.hpp"

namespace terraformer
{
	struct boundary_elevation_modulation
	{
		float offset;
		float base_amplitude;
		float phase;
		float base_frequency;
		int iterations;
	};

	template<class T>
	concept stop_condition = requires(T val, location loc)
	{
		{val(loc)} -> std::same_as<bool>;
	};

	template<stop_condition StopCondition>
	struct ridge_curve_params
	{
		particle_state initial_state;
		noisy_drift::params drift;
		turn_angle_limiter::params turn_angle_limits;
		damped_motion_integrator::params curve_properties;
		StopCondition should_stop;
	};

	template<class Rng, stop_condition StopCondition>
	auto make_ridge_curve(Rng rng, ridge_curve_params<StopCondition> const& params)
	{
		noisy_drift drift{params.drift};
		turn_angle_limiter limiter{
			turn_angle_limiter::state{
				.r = params.initial_state.r,
				.dir = direction{params.initial_state.v},
				.integrated_heading_change = geosimd::turn_angle{0x0}
			},
			params.turn_angle_limits
		};
		damped_motion_integrator integrator{params.initial_state.v, params.curve_properties};

		std::vector<location> ret;
		particle_state state{params.initial_state};
		while(!params.should_stop(state))
		{
			auto const v = drift(rng);
			auto const state_new = integrator(state, v);
			auto const r_corrected = limiter(state_new.r);

			auto const v_corr = 2.0f*(r_corrected - state.r)/
				params.curve_properties.curve_scaling_factor - state.v;

			state.v = v_corr;
			state.r = r_corrected;

			ret.push_back(r_corrected);
		}

		return ret;
	}
}
#endif