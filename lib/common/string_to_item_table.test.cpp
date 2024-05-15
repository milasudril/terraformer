//@	{"target":{"name":"string_to_item_table.test"}}

#include "./string_to_item_table.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_string_to_item_table_direct_insert_and_find)
{
	terraformer::string_to_item_table<int, true> values{
		"key 1",
		13432
	};

	// Finding first item
	{
		auto const res = values.at_ptr("key 1");
		REQUIRE_NE(res, nullptr);
		EXPECT_EQ(*res, 13432);
	}

	// Try to replace first item with `insert`
	{
		auto const res = values.insert("key 1", 545);
		EXPECT_EQ(res.second, false);
		auto const find_res = values.at_ptr("key 1");
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 13432);
	}

	// Replace first item with insert_or_assign
	{
		auto const res = values.insert_or_assign("key 1", 545);
		EXPECT_EQ(res.second, false);
		auto const find_res = values.at_ptr("key 1");
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 545);
	}

	// Try to find a non-existing item
	{
		auto const res = values.at_ptr("key 2");
		EXPECT_EQ(res, nullptr);
	}

	// Insert a new item with insert
	{
		EXPECT_EQ(values.at_ptr("key 3"), nullptr);
		auto const res = values.insert("key 3", 456);
		EXPECT_EQ(res.second, true);
		auto const find_res = values.at_ptr("key 3");
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 456);
	}

	// Try to replace existing item with insert
	{
		EXPECT_NE(values.at_ptr("key 3"), nullptr);
		auto const res = values.insert("key 3", 777);
		EXPECT_EQ(res.second, false);
		auto const find_res = values.at_ptr("key 3");
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 456);
	}

	// Insert new item with insert_or_assign
	{
		EXPECT_EQ(values.at_ptr("key 2"), nullptr);
		auto const res = values.insert_or_assign("key 2", 634);
		EXPECT_EQ(res.second, true);
		auto const find_res = values.at_ptr("key 2");
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 634);
	}

	// Replace existing item (not first one) with insert_or_assign
	{
		EXPECT_NE(values.at_ptr("key 3"), nullptr);
		auto const res = values.insert_or_assign("key 3", 777);
		EXPECT_EQ(res.second, false);
		auto const find_res = values.at_ptr("key 3");
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 777);
	}
}