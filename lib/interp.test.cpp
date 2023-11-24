//@	{"target":{"name":"interp.test"}}

#include "./interp.hpp"
#include "./boundary_sampling_policies.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_interp_wrap_around)
{
	std::array<float, 8> vals{
		0.0f,
		0.125f,
		0.25f,
		0.375f,
		0.5f,
		0.625f,
		0.75f,
		0.875f,
	};

	for(int k = -16; k != 32; ++k)
	{
		auto const val = terraformer::interp(vals, 0.5f*static_cast<float>(k), terraformer::wrap_around_at_boundary{});

		if(k != -1 && k != 15 && k != 31)
		{ EXPECT_EQ(val, 0.5f*static_cast<float>((k + 16)%16)/8.0f); }
		else
		{ EXPECT_EQ(val, 0.5f*(vals[0] + vals[7])); }
	}
}

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