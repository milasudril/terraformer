//@	{"target":{"name":"array_index.test"}}

#include "./array_index.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_array_size_default_constructed)
{
	terraformer::array_size<int> val;
	EXPECT_EQ(val.get(), 0);
}

TESTCASE(terraformer_array_size_constructed_from_value)
{
	terraformer::array_size<int> val{1243};
	EXPECT_EQ(val.get(), 1243);
}

TESTCASE(terraformer_array_size_add_assign)
{
	terraformer::array_size<int> val{124};
	val += terraformer::array_size<int>{23};
	EXPECT_EQ(val.get(), 147);
	
	try
	{
		val += terraformer::array_size<int>{static_cast<size_t>(-1)};
		abort();
	}
	catch(...)
	{}
}

TESTCASE(terraformer_array_size_sub_assign)
{
	terraformer::array_size<int> val{124};
	val -= terraformer::array_size<int>{23};
	EXPECT_EQ(val.get(), 101);
	
	try
	{
		val -= terraformer::array_size<int>{static_cast<size_t>(125)};
		abort();
	}
	catch(...)
	{}
}

TESTCASE(terraformer_array_size_mul_assign)
{
	terraformer::array_size<int> val{124};
	val *= 23;
	EXPECT_EQ(val.get(), 2852);
	
	try
	{
		val *= static_cast<size_t>(-1);
		abort();
	}
	catch(...)
	{}
}

TESTCASE(terraformer_array_size_add)
{
	auto const val = terraformer::array_size<int>{124} + terraformer::array_size<int>{23};
	EXPECT_EQ(val.get(), 147);
}

TESTCASE(terraformer_array_size_sub)
{
	auto const val = terraformer::array_size<int>{124} - terraformer::array_size<int>{23};
	EXPECT_EQ(val.get(), 101);
}

TESTCASE(terraformer_array_size_mul)
{
	auto const val = static_cast<size_t>(23)*terraformer::array_size<int>{124};
	EXPECT_EQ(val.get(), 2852);
}



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

TESTCASE(terraformer_array_index_compare)
{
	EXPECT_EQ(terraformer::array_index<int>{34}, terraformer::array_index<int>{34});

	EXPECT_LT(terraformer::array_index<int>{33}, terraformer::array_index<int>{34});

	EXPECT_NE(terraformer::array_index<int>{35}, terraformer::array_index<int>{34});
}

TESTCASE(terraformer_array_index_compare_to_size)
{
	EXPECT_LE(terraformer::array_index<int>{34}, terraformer::array_index<int>{35});

	EXPECT_NE(terraformer::array_index<int>{34}, terraformer::array_index<int>{35});
}
