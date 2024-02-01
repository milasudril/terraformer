//@	{"target":{"name":"array_index.test"}}

#include "./array_index.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_array_index_default_constructed)
{
	terraformer::array_index<int> val;
	EXPECT_EQ(val.get(), 0);
}

TESTCASE(terraformer_array_index_constructed_from_value)
{
	terraformer::array_index<int> val{34};
	EXPECT_EQ(val.get(), 34);
}

TESTCASE(terraformer_array_index_add_assign)
{
	terraformer::array_index<int> val{34};
	val += 3;
	EXPECT_EQ(val.get(), 37);
}

TESTCASE(terraformer_array_index_subtract_assign)
{
	terraformer::array_index<int> val{34};
	val -= 3;
	EXPECT_EQ(val.get(), 31);
}

TESTCASE(terraformer_array_index_pre_increment)
{
	terraformer::array_index<int> val{34};
	EXPECT_EQ((++val).get(), 35);
	EXPECT_EQ(val.get(), 35);
}

TESTCASE(terraformer_array_index_post_increment)
{
	terraformer::array_index<int> val{34};
	EXPECT_EQ((val++).get(), 34);
	EXPECT_EQ(val.get(), 35);
}

TESTCASE(terraformer_array_index_pre_decrement)
{
	terraformer::array_index<int> val{34};
	EXPECT_EQ((--val).get(), 33);
	EXPECT_EQ(val.get(), 33);
}

TESTCASE(terraformer_array_index_post_devrement)
{
	terraformer::array_index<int> val{34};
	EXPECT_EQ((val--).get(), 34);
	EXPECT_EQ(val.get(), 33);
}

TESTCASE(terraformer_array_index_add)
{
	terraformer::array_index<int> val{34};
	EXPECT_EQ((val + 3).get(), 37);
	EXPECT_EQ(val.get(), 34);
}

TESTCASE(terraformer_array_index_subtract)
{
	terraformer::array_index<int> val{34};
	EXPECT_EQ((val - 3).get(), 31);
	EXPECT_EQ(val.get(), 34);
}

TESTCASE(terraformer_array_index_deref)
{
	std::array<int, 4> vals{1, 2, 3, 4};
	EXPECT_EQ(deref(std::data(vals), terraformer::array_index<int>{3}), 4);
}