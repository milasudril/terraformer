//@	{"target":{"name":"array_tuple.test"}}

#include "./array_tuple.hpp"

#include "testfwk/testfwk.hpp"

#include <iterator>

namespace
{
	using array_tuple_non_copyable =
		terraformer::array_tuple<std::unique_ptr<int>, std::unique_ptr<double>>;

	using array_tulpe_not_trivally_copyable =
		terraformer::array_tuple<std::string, std::vector<int>>;

	using array_tuple_trivially_copyable = terraformer::array_tuple<int, double>;
}

static_assert(std::random_access_iterator<array_tuple_trivially_copyable::const_iterator>);

static_assert(std::random_access_iterator<array_tuple_non_copyable::const_iterator>);

static_assert(std::random_access_iterator<array_tulpe_not_trivally_copyable::const_iterator>);