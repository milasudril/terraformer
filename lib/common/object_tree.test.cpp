//@	{"target":{"name":"object_tree.test"}}

#include "./object_tree.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_object_tree_append)
{
	terraformer::object_tree tree;
	EXPECT_EQ(tree.is_null(), true);

	tree.append<int>(1)
		.append<double>(2.5)
		.append<std::string>("Hello, World");

	REQUIRE_EQ(tree.non_recursive_size(), 3);

	{
		int const* val = tree/0;
		REQUIRE_NE(val, nullptr);
		EXPECT_EQ(*val, 1);
	}

	{
		double const* val = tree/1;
		REQUIRE_NE(val, nullptr);
		EXPECT_EQ(*val, 2.5);
	}

	{
		std::string const* val = tree/2;
		REQUIRE_NE(val, nullptr);
		EXPECT_EQ(*val, "Hello, World");
	}

	EXPECT_EQ((tree/3).is_null(), true);
	EXPECT_EQ(static_cast<int*>(tree/2), nullptr);
}