//@	{"target":{"name":"flat_map.test"}}

#include "./flat_map.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_flat_map_insert)
{
	using map_type =  terraformer::flat_map<std::less<int>, int, double>;
	map_type my_map;

	// Find element in empty map
	{
		auto const find_res = my_map.find(1);
		EXPECT_EQ(find_res, map_type::npos);
	}

	// Insert element and find it again
	{
		auto const res = my_map.insert(1, 1.5);
		EXPECT_EQ(res.first, map_type::index_type{0});
		EXPECT_EQ(res.second, true);
		EXPECT_EQ(my_map.keys()[res.first], 1);
		EXPECT_EQ(my_map.values<0>()[res.first], 1.5);
		auto const find_res = my_map.find(1);
		EXPECT_EQ(res.first, find_res);
	}

	// Try to insert already existing element
	{
		auto const res = my_map.insert(1, 2.5);
		EXPECT_EQ(res.first, map_type::index_type{0});
		EXPECT_EQ(res.second, false);
		EXPECT_EQ(my_map.keys()[res.first], 1);
		EXPECT_EQ(my_map.values<0>()[res.first], 1.5);
		auto const find_res = my_map.find(1);
		EXPECT_EQ(res.first, find_res);
	}

	// Try to find non-existing element (Element is after existing element)
	{
		auto const find_res = my_map.find(2);
		EXPECT_EQ(find_res, map_type::npos);
	}

	// Try to find non-existing element (Element is before existing element)
	{
		auto const find_res = my_map.find(0);
		EXPECT_EQ(find_res, map_type::npos);
	}

	// Insert element last and find it again
	{
		auto const res = my_map.insert(3, 2.5);
		EXPECT_EQ(res.first, map_type::index_type{1});
		EXPECT_EQ(res.second, true);
		EXPECT_EQ(my_map.keys()[res.first], 3);
		EXPECT_EQ(my_map.values<0>()[res.first], 2.5);
		auto const find_res = my_map.find(3);
		EXPECT_EQ(res.first, find_res);
	}

	// Insert element first and find it again
	{
		auto const res = my_map.insert(0, 3.5);
		EXPECT_EQ(res.first, map_type::index_type{0});
		EXPECT_EQ(res.second, true);
		EXPECT_EQ(my_map.keys()[res.first], 0);
		EXPECT_EQ(my_map.values<0>()[res.first], 3.5);
		auto const find_res = my_map.find(0);
		EXPECT_EQ(res.first, find_res);
	}

	// Insert element in the middle and find it again
	{
		auto const res = my_map.insert(2, 4.5);
		EXPECT_EQ(res.first, map_type::index_type{2});
		EXPECT_EQ(res.second, true);
		EXPECT_EQ(my_map.keys()[res.first], 2);
		EXPECT_EQ(my_map.values<0>()[res.first], 4.5);
		auto const find_res = my_map.find(2);
		EXPECT_EQ(res.first, find_res);
	}

	// Check content (keys)
	{
		auto const keys = my_map.keys();
		for(auto k : keys.element_indices())
		{ EXPECT_EQ(keys[k], static_cast<int>(k.get())); }
	}

	// Check content (values, assumes keys were correct)
	{
		auto const values = my_map.values<0>();
		for(auto k : values.element_indices())
		{
			auto res = my_map.find(static_cast<int>(k.get()));
			EXPECT_EQ(res.get(), k.get());
			EXPECT_EQ(values[k], values[res]);
		}
	}

	// Overwrite existing value
	{
		auto const find_res = my_map.find(2);
		EXPECT_NE(find_res, map_type::npos);
		auto const insert_res = my_map.insert_or_assign(2, 6.5);
		EXPECT_EQ(insert_res.first, map_type::index_type{2});
		EXPECT_EQ(my_map.values<0>()[insert_res.first], 6.5);
		EXPECT_EQ(insert_res.second, false);
	}

	// Insert or assign with a new key
	{
		auto const find_res = my_map.find(10);
		EXPECT_EQ(find_res, map_type::npos);
		auto const insert_res = my_map.insert_or_assign(10, 8.0);
		EXPECT_EQ(insert_res.first, map_type::index_type{4});
		EXPECT_EQ(my_map.values<0>()[insert_res.first], 8.0);
		EXPECT_EQ(insert_res.second, true);
	}
}