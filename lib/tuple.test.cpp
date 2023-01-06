//@	{"target":{"name":"tuple.test"}}

#include "./tuple.hpp"

#include "testfwk/testfwk.hpp"

#include <type_traits>

static_assert(std::is_trivially_copyable_v<terraformer::tuple<size_t, int, double>>);
static_assert(std::is_trivially_copy_assignable_v<terraformer::tuple<size_t, int, double>>);
static_assert(sizeof(terraformer::tuple<size_t, int, double>) == 2*sizeof(double));
static_assert(std::equality_comparable<terraformer::tuple<size_t, int, double>>);

TESTCASE(terraformer_tuple_get_values)
{
	terraformer::tuple<size_t, int, double> x{1, 3.0};

	auto [v1, v2] = x;
}
