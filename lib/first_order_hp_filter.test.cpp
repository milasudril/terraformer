//@	{"target":{"name":"first_order_hp_filter.test"}}

#include "./first_order_hp_filter.hpp"

#include <testfwk/testfwk.hpp>

#include <cmath>

TESTCASE(terraformer_first_order_hp_filter_step_response)
{
	terraformer::first_order_hp_filter f{
		terraformer::first_order_hp_filter_description{
			.cutoff_freq = 1.0f,
			.initial_value = 0.0f,
			.initial_input = 0.0f
		},
		1.0f/1024.0f
	};

	auto const init = f(1.0f);
	auto val = init;
	for(size_t k = 0; k != 1024; ++k)
	{
		auto const val_next = f(1.0f);
		EXPECT_LT(val_next, val);
		val = val_next;
	}

	EXPECT_LT(std::abs(init/val - std::exp(1)), 1.0e-6f);
}