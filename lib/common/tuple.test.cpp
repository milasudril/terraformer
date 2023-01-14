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

	{
		auto [v1, v2, v3] = x;
		EXPECT_EQ(v1, 1);
		EXPECT_EQ(v2, 2);
		EXPECT_EQ(v3, 3.0);
	}

	auto const d = get<0>(x);
	EXPECT_EQ(d, 1);

	{
		terraformer::tuple y{1, std::make_unique<int>(2)};
		auto y2_before = get<1>(y).get();
		auto [v1, v2] = std::move(y);
		EXPECT_EQ(reinterpret_cast<intptr_t>(get<1>(y).get()), 0);
		EXPECT_EQ(reinterpret_cast<intptr_t>(v2.get()), reinterpret_cast<intptr_t>(y2_before));
	}

	{
		terraformer::tuple y{1, std::make_unique<int>(2)};
		auto& [v1, v2] = y;
		EXPECT_EQ(reinterpret_cast<intptr_t>(get<1>(y).get()), reinterpret_cast<intptr_t>(v2.get()));
	}
}

TESTCASE(terraformer_tuple_apply)
{
	terraformer::tuple x{1, 2, 3};

	apply([x](auto ... item){
		terraformer::tuple t{item...};
		EXPECT_EQ(get<0>(x), get<0>(t));
		EXPECT_EQ(get<1>(x), get<1>(t));
		EXPECT_EQ(get<2>(x), get<2>(t));
	}, x);
}

#include "./spaces.hpp"

static_assert(
	std::is_same_v<std::tuple_element_t<0, terraformer::tuple<terraformer::location, terraformer::direction>>, terraformer::location >
);

static_assert(
	std::is_same_v<std::tuple_element_t<1, terraformer::tuple<terraformer::location, terraformer::direction>>, terraformer::direction >
);