//@	{"target":{"name":"constants.test"}}

#include "./constants.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(Unity)
{
	constexpr auto val = unity<int>();
	static_assert(std::is_same_v<decltype(val), int const>);
	EXPECT_EQ(val, 1);
}

TESTCASE(Zero)
{
	constexpr auto val = zero<int>();
	static_assert(std::is_same_v<decltype(val), int const>);
	EXPECT_EQ(val, 0);
}