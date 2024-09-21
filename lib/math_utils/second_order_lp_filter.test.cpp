//@	{"target":{"name":"second_order_lp_filter.test"}}

#include "./second_order_lp_filter.hpp"

#include <testfwk/testfwk.hpp>

#include <array>

TESTCASE(terraformer_second_order_lp_filter_resonant_step_response)
{
	terraformer::second_order_lp_filter f{
		terraformer::second_order_lp_filter_description{
			.damping = 0.125f,
			.cutoff_freq = 2.0f*std::numbers::pi_v<float>,
			.initial_value = 0.0f,
			.initial_derivative = 0.0f,
			.initial_input = 0.0f
		}
	};

	std::array<float, 256> vals{};
	for(size_t k = 0; k != 256; ++k)
	{
		vals[k] = f(1.0f, 1.0f/16.0f);
	}

	EXPECT_GT(vals[8], vals[7]);
	EXPECT_GT(vals[8], vals[9]);

	EXPECT_LT(vals[16], vals[15]);
	EXPECT_LT(vals[16], vals[17]);
	EXPECT_GT(vals[255], 0.975f);
	EXPECT_LT(vals[255], 1.125f);
}