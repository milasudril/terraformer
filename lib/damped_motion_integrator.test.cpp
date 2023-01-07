//@	{"target":{"name":"damped_motion_integrator.test"}}

#include "./damped_motion_integrator.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_damped_motion_integrator_step_response_at_min_inertia)
{
	constexpr terraformer::displacement v{1.0f, 0.0f, 0.0f};
	terraformer::particle_state state{};
	terraformer::damped_motion_integrator integrator{v,
		terraformer::damped_motion_integrator::params{
			.inertia = terraformer::damped_motion_integrator::min_inertia(),
			.curve_scaling_factor = 1.0f
		}
	};

	state = integrator(state, v);
	EXPECT_EQ(state.v, v);
	state = integrator(state, v);
	EXPECT_EQ(state.v, v);
}