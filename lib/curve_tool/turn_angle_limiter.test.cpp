//@	{"target":{"name":"turn_angle_limiter.test"}}

#include "./turn_angle_limiter.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_turn_angle_limiter_no_clamp)
{
	terraformer::turn_angle_limiter::params const params{
		.max_turn_angle = geosimd::turn_angle{0x8000'0000}
	};

	terraformer::turn_angle_limiter::state const initial_state{
		.r = terraformer::location{1.0f, 0.0f, 0.0f},
		.dir = terraformer::direction{terraformer::geom_space::x{}},
		.integrated_heading_change = geosimd::turn_angle{0x0}
	};

	terraformer::turn_angle_limiter limiter{initial_state, params};

	terraformer::location new_loc{1.0f, 1.0f, 0.0f};
	auto const res = limiter(new_loc);
	EXPECT_EQ(res, new_loc);
}

TESTCASE(terraformer_turn_angle_limiter_clamp_at_max)
{
	terraformer::turn_angle_limiter::params const params{
		.max_turn_angle = geosimd::turn_angle{0x2000'0000}
	};

	terraformer::turn_angle_limiter::state const initial_state{
		.r = terraformer::location{1.0f, 0.0f, 0.0f},
		.dir = terraformer::direction{terraformer::geom_space::x{}},
		.integrated_heading_change = geosimd::turn_angle{0x0}
	};

	terraformer::turn_angle_limiter limiter{initial_state, params};

	terraformer::location new_loc{1.0f, 1.0f, 0.0f};
	auto const res = limiter(new_loc);
	EXPECT_LT(distance(res, terraformer::location{1.5f, 0.5f, 0.0f}), 1e-7f);
}

TESTCASE(terraformer_turn_angle_limiter_clamp_at_min)
{
	terraformer::turn_angle_limiter::params const params{
		.max_turn_angle = geosimd::turn_angle{0x2000'0000}
	};

	terraformer::turn_angle_limiter::state const initial_state{
		.r = terraformer::location{1.0f, 0.0f, 0.0f},
		.dir = terraformer::direction{terraformer::geom_space::x{}},
		.integrated_heading_change = geosimd::turn_angle{0x0}
	};

	terraformer::turn_angle_limiter limiter{initial_state, params};

	terraformer::location new_loc{1.0f, -1.0f, 0.0f};
	auto const res = limiter(new_loc);
	EXPECT_LT(distance(res, terraformer::location{1.5f, -0.5f, 0.0f}), 1e-7f);
}

TESTCASE(terraformer_turn_angle_limiter_clamp_going_back_prevented_1)
{
	terraformer::turn_angle_limiter::params const params{
		.max_turn_angle = geosimd::turn_angle{0x4000'0000}
	};

	terraformer::turn_angle_limiter::state const initial_state{
		.r = terraformer::location{0.0f, 0.0f, 0.0f},
		.dir = -terraformer::direction{terraformer::geom_space::x{}},
		.integrated_heading_change = geosimd::turn_angle{0x0}
	};

	terraformer::turn_angle_limiter limiter{initial_state, params};

	auto res = limiter(terraformer::location{0.0f, -1.0f, 0.0f});
	EXPECT_LT(distance(res, terraformer::location{0.0f, -1.0f, 0.0f}), 1e-7f);
	res = limiter(terraformer::location{1.0f, -2.0f, 0.0f});
	EXPECT_LT(distance(res, terraformer::location{0.0f, -2.0f, 0.0f}), 1e-7f);
}

TESTCASE(terraformer_turn_angle_limiter_clamp_going_back_prevented_2)
{
	terraformer::turn_angle_limiter::params const params{
		.max_turn_angle = geosimd::turn_angle{0x4000'0000}
	};

	terraformer::turn_angle_limiter::state const initial_state{
		.r = terraformer::location{0.0f, 0.0f, 0.0f},
		.dir = -terraformer::direction{terraformer::geom_space::x{}},
		.integrated_heading_change = geosimd::turn_angle{0x0}
	};

	terraformer::turn_angle_limiter limiter{initial_state, params};

	auto res = limiter(terraformer::location{0.0f, 1.0f, 0.0f});
	EXPECT_LT(distance(res, terraformer::location{0.0f, 1.0f, 0.0f}), 1e-7f);
	res = limiter(terraformer::location{1.0f, 2.0f, 0.0f});
	EXPECT_LT(distance(res, terraformer::location{0.0f, 2.0f, 0.0f}), 1e-7f);
}