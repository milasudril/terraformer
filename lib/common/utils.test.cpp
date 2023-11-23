//@	{"target":{"name":"utils.test"}}

#include "./utils.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_round_to_n_digits)
{
	{
		auto res = terraformer::round_to_n_digits(512.0f, 1);
		EXPECT_EQ(res, 512.0f);
	}

	{
		auto res = terraformer::round_to_n_digits(900.0f, 1);
		EXPECT_EQ(res, 1024.0f);
	}
}

TESTCASE(teraformer_round_to_odd)
{
	{
		auto res = terraformer::round_to_odd(0.0f);
		EXPECT_EQ(res, 1);
	}

	{
		auto res = terraformer::round_to_odd(0.5f);
		EXPECT_EQ(res, 1);
	}

	{
		auto res = terraformer::round_to_odd(1.5f);
		EXPECT_EQ(res, 1);
	}

	{
		auto res = terraformer::round_to_odd(2.0f);
		EXPECT_EQ(res, 3);
	}

	{
		auto res = terraformer::round_to_odd(2.5f);
		EXPECT_EQ(res, 3);
	}

	{
		auto res = terraformer::round_to_odd(3.0f);
		EXPECT_EQ(res, 3);
	}
}