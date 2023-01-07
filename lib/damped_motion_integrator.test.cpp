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
	EXPECT_EQ(state.r, (terraformer::location{0.5f, 0.0f, 0.0f}));
	state = integrator(state, v);
	EXPECT_EQ(state.v, v);
	EXPECT_EQ(state.r, (terraformer::location{1.5f, 0.0f, 0.0f}));
	state = integrator(state, v);
	EXPECT_EQ(state.v, v);
	EXPECT_EQ(state.r, (terraformer::location{2.5f, 0.0f, 0.0f}));
}

TESTCASE(terraformer_damped_motion_integrator_step_response_inertia_1dot5)
{
	constexpr terraformer::displacement v{1.0f, 0.0f, 0.0f};
	terraformer::particle_state state{};
	terraformer::damped_motion_integrator integrator{v,
		terraformer::damped_motion_integrator::params{
			.inertia = 1.5f,
			.curve_scaling_factor = 1.0f
		}
	};

	auto exponent = 2.0f;
	for(size_t k = 0; k != 8; ++k)
	{
		state = integrator(state, v);
		auto [vx, vy, vz] = state.v;
		EXPECT_EQ(vx, (1.0f - 1.0f/exponent));
		EXPECT_EQ(vy, 0.0f);
		EXPECT_EQ(vz, 0.0f);
		exponent *= 2.0f;
	}
}