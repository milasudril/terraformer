//@	{"target":{"name":"resource_table.test"}}

#include "./resource_table.hpp"

#include <testfwk/testfwk.hpp>

#include <string>

TESTCASE(terraformer_resource_table_default_state)
{
	terraformer::resource_table<std::string, int> table;
	EXPECT_EQ(table.contains("Hej"), false);
	EXPECT_EQ(table.at_ptr("Hej"), nullptr);
	EXPECT_EQ(std::as_const(table).contains("Hej"), false);
	EXPECT_EQ(std::as_const(table).at_ptr("Hej"), nullptr);

	auto const link = table.create_link("Hej", "New key");
	EXPECT_EQ(link.first, nullptr);
	EXPECT_EQ(link.second, false);
}

TESTCASE(terraformer_resource_table_emplace)
{
	terraformer::resource_table<std::string, int> table;
	REQUIRE_EQ(std::as_const(table).contains("Hej"), false);
	REQUIRE_EQ(std::as_const(table).at_ptr("Hej"), nullptr);

	auto const res1 = table.emplace("Hej", 123);

	EXPECT_EQ(res1.second, true);
	REQUIRE_NE(res1.first, nullptr);
	EXPECT_EQ(*res1.first, 123);
	REQUIRE_EQ(std::as_const(table).contains("Hej"), true);
	EXPECT_EQ(std::as_const(table).at_ptr("Hej"), res1.first);

	auto const res2 = table.emplace("Hej", 456);
	EXPECT_EQ(res2.first, res1.first);
	EXPECT_EQ(res2.second, false);

	auto const link = table.create_link("Hej", "New key");
	EXPECT_EQ(link.first, res1.first);
	EXPECT_EQ(link.second, true);

	auto const link2 = table.create_link("Hej", "New key");
	EXPECT_EQ(link2.first, link.first);
	EXPECT_EQ(link2.second, false);

	auto const res3 = table.emplace("New key", 456);
	EXPECT_EQ(res3.first, res1.first);

	auto const res5 = table.insert_or_assign_linked("Hej", 789);
	REQUIRE_NE(res5.first, nullptr);
	EXPECT_EQ(*res5.first, 789);
	EXPECT_EQ(res5.second, false);
	EXPECT_EQ(res5.first, link.first);

	auto const res6 = table.insert_or_assign_linked("Kaka", 23243);
	REQUIRE_NE(res6.first, nullptr);
	EXPECT_EQ(*res6.first, 23243);
	EXPECT_EQ(res6.second, true);
}