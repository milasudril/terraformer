//@	{"target":{"name":"adj_algo.test"}}

#include "./adj_algo.hpp"

#include "testfwk/testfwk.hpp"

#include <array>
#include <utility>
#include <type_traits>

TESTCASE(AdjForEach)
{
	std::array<int, 4> vals{1, 2, 3, 4};
	std::array<std::pair<int, int>, 3> visited{};
	int k = 0;
	adj_for_each(std::begin(vals), std::end(vals), [&visited, &k](auto a, auto b) {
		visited[k] = std::make_pair(a, b);
		++k;
	});
	REQUIRE_EQ(k, 3);
	std::array<std::pair<int, int>, 3> expected;
	expected[0] = std::make_pair(1, 2);
	expected[1] = std::make_pair(2, 3);
	expected[2] = std::make_pair(3, 4);
	EXPECT_EQ(visited, expected);
}

TESTCASE(AdjIntegrate)
{
	std::array<float, 4> vals{1.0f, 2.0f, 3.0f, 4.0f};
	auto res = adj_integrate(std::begin(vals), std::end(vals), [](auto a, auto b) {
		return a - b;
	}, static_cast<int>(0));

	static_assert(std::is_same_v<decltype(res), decltype(vals)::value_type>);
	EXPECT_EQ(res, (1.0f - 2.0f) + (2.0f - 3.0f) + (3.0f - 4.0f));
}

TESTCASE(AdjTransform)
{
	std::array<int, 4> vals{1, 2, 3, 4};
	std::array<int, 3> visited{};

	adj_transform(std::begin(vals), std::end(vals), std::begin(visited),[](auto a, auto b){
		return a + b;
	});

	std::array<int, 3> expected{3, 5, 7};

	EXPECT_EQ(visited, expected);
}