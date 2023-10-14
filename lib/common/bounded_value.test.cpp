//@	{"target":{"name": "bounded_value.test"}}

#include "./bounded_value.hpp"
#include "./interval.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_bounded_value_default)
{
	using zero_to_one_value = terraformer::bounded_value<terraformer::closed_closed_interval{0.0f, 1.0f}, 0.5f>;

	zero_to_one_value my_value;
	EXPECT_EQ(my_value, 0.5f);
	EXPECT_EQ(0.5f, my_value);
	auto str = to_string(my_value);
	EXPECT_EQ(str, "0.5");

	static_assert(std::is_same_v<zero_to_one_value::value_type, float>);
	static_assert(zero_to_one_value::min() == 0.0f);
	static_assert(zero_to_one_value::max() == 1.0f);
	static_assert(zero_to_one_value::default_value() == 0.5f);
}

TESTCASE(terraformer_bounded_value_construct_within_range)
{
	using zero_to_one_value = terraformer::bounded_value<terraformer::closed_closed_interval{0.0f, 1.0f}, 0.5f>;

	zero_to_one_value my_value{0.75f};
	EXPECT_EQ(my_value, 0.75f);
	EXPECT_EQ(0.75f, my_value);
	auto str = to_string(my_value);
	EXPECT_EQ(str, "0.75");

	static_assert(std::is_same_v<zero_to_one_value::value_type, float>);
}

TESTCASE(terraformer_bounded_value_construct_out_of_range)
{
	using zero_to_one_value = terraformer::bounded_value<terraformer::closed_closed_interval{0.0f, 1.0f}, 0.5f>;

	try
	{
		zero_to_one_value my_value{2.0f};
		abort();
	}
	catch(...)
	{}
}