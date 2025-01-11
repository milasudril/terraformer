//@	{"target":{"name":"interp.test"}}

#include "./interp.hpp"
#include "./boundary_sampling_policies.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_interp_eqdist_wrap_around)
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
		auto const val = terraformer::interp_eqdist(vals, 0.5f*static_cast<float>(k), terraformer::wrap_around_at_boundary{});

		if(k != -1 && k != 15 && k != 31)
		{ EXPECT_EQ(val, 0.5f*static_cast<float>((k + 16)%16)/8.0f); }
		else
		{ EXPECT_EQ(val, 0.5f*(vals[0] + vals[7])); }
	}
}

TESTCASE(terraformer_interp_clamp)
{
	std::array<float, 4> x_vals{0.0f, 0.5f, 0.75f, 1.0f};
	std::array<float, 4> y_vals{0.f, 1.0f/3.0f, 2.0f/3.0f, 1.0f};

	EXPECT_EQ(terraformer::interp(x_vals, y_vals, 0.25f), 1.0f/6.0f);

	EXPECT_EQ(terraformer::interp(x_vals, y_vals, 0.75f), 2.0f/3.0f);

	EXPECT_EQ(terraformer::interp(x_vals, y_vals, 0.0f), 0.0f);

	EXPECT_EQ(terraformer::interp(x_vals, y_vals, 1.0f), 1.0f);

	EXPECT_EQ(terraformer::interp(x_vals, y_vals, -0.25f), 0.0f);

	EXPECT_EQ(terraformer::interp(x_vals, y_vals, 1.25f), 1.0f);
}