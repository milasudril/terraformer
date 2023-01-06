//@	{"target":{"name":"tuple.test"}}

#include "./tuple.hpp"

#include "testfwk/testfwk.hpp"

#include <type_traits>

static_assert(std::is_trivial_v<terraformer::tuple<size_t, int, double>>);
static_assert(sizeof(terraformer::tuple<size_t, int, double>) == 2*sizeof(double));
static_assert(std::equality_comparable<terraformer::tuple<size_t, int, double>>);
