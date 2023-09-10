//@	{"target":{"name":"interval.test"}}

#include "./interval.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_open_open_interval)
{
	terraformer::open_open_interval i{1, 3};
	EXPECT_EQ(within(i, 0), false);
	EXPECT_EQ(within(i, 1), false);
	EXPECT_EQ(within(i, 2), true);
	EXPECT_EQ(within(i, 3), false);
	EXPECT_EQ(within(i, 4), false);

	auto str = to_string(i);
	EXPECT_EQ(str, "]1, 3[");
}

TESTCASE(terraformer_open_closed_interval)
{
	terraformer::open_closed_interval i{1, 3};
	EXPECT_EQ(within(i, 0), false);
	EXPECT_EQ(within(i, 1), false);
	EXPECT_EQ(within(i, 2), true);
	EXPECT_EQ(within(i, 3), true);
	EXPECT_EQ(within(i, 4), false);
	auto str = to_string(i);
	EXPECT_EQ(str, "]1, 3]");
}

TESTCASE(terraformer_closed_open_interval)
{
	terraformer::open_open_interval i{1, 3};
	EXPECT_EQ(within(i, 0), false);
	EXPECT_EQ(within(i, 1), true);
	EXPECT_EQ(within(i, 2), true);
	EXPECT_EQ(within(i, 3), false);
	EXPECT_EQ(within(i, 4), false);

	auto str = to_string(i);
	EXPECT_EQ(str, "[1, 3[");
}

TESTCASE(terraformer_closed_closed_interval)
{
	terraformer::open_closed_interval i{1, 3};
	EXPECT_EQ(within(i, 0), false);
	EXPECT_EQ(within(i, 1), true);
	EXPECT_EQ(within(i, 2), true);
	EXPECT_EQ(within(i, 3), true);
	EXPECT_EQ(within(i, 4), false);
	auto str = to_string(i);
	EXPECT_EQ(str, "[1, 3]");
}