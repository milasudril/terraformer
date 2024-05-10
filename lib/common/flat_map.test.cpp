//@	{"target":{"name":"flat_map.test"}}

#include "./flat_map.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_flat_map_insert)
{
	using map_type =  terraformer::flat_map<std::less<int>, int, double>;
	map_type my_map;

	{
		auto const res = my_map.insert(1, 1.5);
		EXPECT_EQ(res.first, map_type::index_type{0});
		EXPECT_EQ(res.second, true);
		EXPECT_EQ(my_map.keys()[res.first], 1);
		EXPECT_EQ(my_map.values<0>()[res.first], 1.5);
		auto const find_res = my_map.find(1);
		EXPECT_EQ(res.first, find_res);
	}

	{
		auto const res = my_map.insert(1, 2.5);
		EXPECT_EQ(res.first, map_type::index_type{0});
		EXPECT_EQ(res.second, false);
		EXPECT_EQ(my_map.keys()[res.first], 1);
		EXPECT_EQ(my_map.values<0>()[res.first], 1.5);
		auto const find_res = my_map.find(1);
		EXPECT_EQ(res.first, find_res);
	}
}