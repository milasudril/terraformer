//@	{"target":{"name":"cubic_spline.test"}}

#include "./cubic_spline.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_interp_cubic_spline)
{
	terraformer::cubic_spline_control_point a{
		.y = 0.0f,
		.ddx = -1.0f
	};

	terraformer::cubic_spline_control_point b{
		.y = 1.0f,
		.ddx = -2.0f
	};

	EXPECT_EQ(interp(a, b, 0.0f), a.y);
	EXPECT_EQ(interp(a, b, 1.0f), b.y);

	// TODO: Check endpoint derivatives somehow
}