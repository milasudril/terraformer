//@	{"target":{"name":"spline.test"}}

#include "./spline.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_smoothstep)
{
	EXPECT_EQ(terraformer::smoothstep(-2.0f), 0.0f);
	EXPECT_EQ(terraformer::smoothstep(-1.0f), 0.0f);
	EXPECT_EQ(terraformer::smoothstep(0.0f), 0.5f);
	EXPECT_EQ(terraformer::smoothstep(1.0f), 1.0f);
	EXPECT_EQ(terraformer::smoothstep(2.0f), 1.0f);
}

TESTCASE(terraformer_spline)
{
	terraformer::spline_control_point a{
		.y = 0.0f,
		.ddx = -1.0f
	};

	terraformer::spline_control_point b{
		.y = 1.0f,
		.ddx = -2.0f
	};

	EXPECT_EQ(spline(0.0f, a, b), a.y);
	EXPECT_EQ(spline(1.0f, a, b), b.y);

	// TODO: Check endpoint derivatives somehow
}