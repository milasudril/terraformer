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

TESTCASE(terraformer_smoothstep)
{
	EXPECT_EQ(terraformer::smoothstep(-2.0f), 0.0f);
	EXPECT_EQ(terraformer::smoothstep(-1.0f), 0.0f);
	EXPECT_EQ(terraformer::smoothstep(0.0f), 0.5f);
	EXPECT_EQ(terraformer::smoothstep(1.0f), 1.0f);
	EXPECT_EQ(terraformer::smoothstep(2.0f), 1.0f);
}

TESTCASE(terraformer_cubic_polynomial_with_crit_point_descriptor_check_polynomial)
{
	terraformer::cubic_polynomial_with_crit_point_descriptor poly
	{
		.x_crit = terraformer::bounded_value<terraformer::open_open_interval{0.0f, 1.0e+0f}, 5.0e-1f>{0.5f},
		.y_0 = 1.0f,
		.y_crit = 3.0f,
		.y_1 = 2.0f
	};

	auto const p = make_polynomial(poly);

	EXPECT_EQ(p(0.0f), 1.0f);
	EXPECT_EQ(p(0.5f), 3.0f);
	EXPECT_EQ(p(1.0f), 2.0f);

	auto const dp = p.derivative();
	EXPECT_EQ(dp(0.5f), 0.0f);
}