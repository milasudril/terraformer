//@	{"target":{"name":"resource_table.test"}}

#include "./resource_table.hpp"

#include <testfwk/testfwk.hpp>

#include <string>

TESTCASE(terraformer_resource_table_default_state)
{
	terraformer::resource_table table;
	EXPECT_EQ(table.contains("Hej"), false);
	EXPECT_EQ(table.get_if<int>("Hej"), nullptr);
	EXPECT_EQ(std::as_const(table).contains("Hej"), false);
	EXPECT_EQ(std::as_const(table).get_if<int>("Hej"), nullptr);

	auto const link_res = table.create_link("Hej", "New key");
	EXPECT_EQ(link_res, false);
}

TESTCASE(terraformer_resource_table_emplace)
{
	terraformer::resource_table table;
	REQUIRE_EQ(std::as_const(table).contains("Hej"), false);
	REQUIRE_EQ(std::as_const(table).get_if<int>("Hej"), nullptr);

	auto const res1 = table.emplace<int>("Hej", 123);

	EXPECT_EQ(res1.second, true);
	REQUIRE_NE(res1.first, nullptr);
	EXPECT_EQ(*res1.first, 123);
	REQUIRE_EQ(std::as_const(table).contains("Hej"), true);
	EXPECT_EQ(std::as_const(table).get_if<int>("Hej"), res1.first);
	EXPECT_EQ(std::as_const(table).get_if<double>("Hej"), nullptr);

	auto const res2 = table.emplace<int>("Hej", 456);
	EXPECT_EQ(res2.first, res1.first);
	EXPECT_EQ(res2.second, false);

	auto const link_res_1 = table.create_link("Hej", "New key");
	EXPECT_EQ(link_res_1, true);

	auto const link_res_2 = table.create_link("Hej", "New key");
	EXPECT_EQ(link_res_2, false);

	auto const res3 = table.emplace<int>("New key", 456);
	EXPECT_EQ(res3.first, res1.first);

	auto const res4 = table.insert_or_assign_linked<int>("Hej", 789);
	REQUIRE_NE(res4.first, nullptr);
	EXPECT_EQ(*res4.first, 789);
	EXPECT_EQ(res4.second, false);
	EXPECT_EQ(res4.first, table.get_if<int>("New key"));

	auto const res5 = table.insert_or_assign_linked<double>("Hej", 789.0);
	EXPECT_EQ(res5.first, nullptr);
	EXPECT_EQ(res5.second, false);

	auto const res6 = table.insert_or_assign_linked<int>("Kaka", 23243);
	REQUIRE_NE(res6.first, nullptr);
	EXPECT_NE(res6.first, res4.first);
	EXPECT_EQ(*res6.first, 23243);
	EXPECT_EQ(res6.second, true);
}