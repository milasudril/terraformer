//@	{"target":{"name":"object_tree.test"}}

#include "./object_tree.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_object_tree_append)
{
	terraformer::object_tree tree;
	EXPECT_EQ(tree.is_null(), true);
	tree.append<int>(1)
		.append<double>(2.5)
		.append<std::string>("Hello, World")
		.append<terraformer::object_tree::array_type>(
			terraformer::object_array{}
				.append<int>(3)
		);

	REQUIRE_EQ(tree.non_recursive_size(), 4);

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

	{
		terraformer::object_array* val = tree/3;
		REQUIRE_NE(val, nullptr);
		REQUIRE_NE(std::size(*val), terraformer::object_array::size_type{0});

		int const* inner_val = (*val)/0;
		REQUIRE_NE(inner_val, nullptr);
		EXPECT_EQ(*inner_val, 3);

		int const* inner_val_2 = tree/3/0;
		REQUIRE_NE(inner_val_2, nullptr);
		EXPECT_EQ(inner_val_2, inner_val);

		int const* inner_val_3 = tree/3/1;
		EXPECT_EQ(inner_val_3, nullptr);
	}

	EXPECT_EQ((tree/4).is_null(), true);
	EXPECT_EQ(static_cast<int*>(tree/2), nullptr);
}