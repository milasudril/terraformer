//@	{"target":{"name":"tuple.test"}}

#include "./tuple.hpp"

#include "testfwk/testfwk.hpp"

#include <type_traits>

static_assert(std::is_trivially_copyable_v<terraformer::tuple<int, double>>);
static_assert(std::is_trivially_copy_assignable_v<terraformer::tuple<int, double>>);
static_assert(sizeof(terraformer::tuple<int, double>) == 2*sizeof(double));
static_assert(std::equality_comparable<terraformer::tuple<int, double>>);

TESTCASE(terraformer_tuple_get_values)
{
	terraformer::tuple x{1, 2, 3.0};

	auto [v1, v2, v3] = x;
	EXPECT_EQ(v1, 1);
	EXPECT_EQ(v2, 2);
	EXPECT_EQ(v3, 3.0);
}
