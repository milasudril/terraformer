//@	{"target":{"name": "bounded_value.test"}}

#include "./bounded_value.hpp"
#include "./interval.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_bounded_value_test)
{
	using zero_to_one_value = terraformer::bounded_value<terraformer::closed_closed_interval{0.0f, 1.0f}, 0.5f>;

	zero_to_one_value my_value;
	EXPECT_EQ(my_value, 0.5f);
	EXPECT_EQ(0.5f, my_value);
	auto str = to_string(my_value);
	EXPECT_EQ(str, "0.5");

	static_assert(std::is_same_v<zero_to_one_value::value_type, float>);
}