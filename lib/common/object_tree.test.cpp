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

	EXPECT_EQ(tree.insert<int>("An array cannot take string as key", 345).is_null(), true);
	EXPECT_EQ(tree.non_recursive_size(), 4);

	EXPECT_EQ((tree/"Trying to use a string as key").is_null(), true);

	tree.insert_or_assign<int>("One", 35434);
	EXPECT_EQ(tree.non_recursive_size(), 4);
	EXPECT_EQ(*static_cast<int*>(tree/0), 1);
}

TESTCASE(terraformer_object_tree_insert)
{
	terraformer::object_tree tree;
	EXPECT_EQ(tree.is_null(), true);
	tree.insert<int>("One", 1)
		.insert<double>("Two", 2.5)
		.insert<std::string>("Three", "Hello, World")
		.insert<terraformer::object_array>(
			"An array",
			terraformer::object_array{}
				.append<int>(3)
				.append<terraformer::object_dict>(
					terraformer::object_dict{}
						.insert_or_assign<double>("A value", 6.5)
						.insert<double>("Another value", 4.5)
					)
			);

	REQUIRE_EQ(tree.non_recursive_size(), 4);
	{
		int const* val = tree/"One";
		REQUIRE_NE(val, nullptr);
		EXPECT_EQ(*val, 1);
	}

	{
		double const* val = tree/"Two";
		REQUIRE_NE(val, nullptr);
		EXPECT_EQ(*val, 2.5);
	}

	{
		std::string const* val = tree/"Three";
		REQUIRE_NE(val, nullptr);
		EXPECT_EQ(*val, "Hello, World");
	}

	{
		terraformer::object_array* val = tree/"An array";
		REQUIRE_NE(val, nullptr);
		REQUIRE_NE(std::size(*val), terraformer::object_array::size_type{0});
		int const* inner_val = (*val)/0;
		REQUIRE_NE(inner_val, nullptr);
		EXPECT_EQ(*inner_val, 3);

		int const* inner_val_2 = tree/"An array"/0;
		REQUIRE_NE(inner_val_2, nullptr);
		EXPECT_EQ(inner_val_2, inner_val);

		double const* inner_val_3 = tree/"An array"/1/"A value";
		REQUIRE_NE(inner_val_3, nullptr);
		EXPECT_EQ(*inner_val_3, 6.5);

		double const* inner_val_4 = tree/"An array"/1/"Another value";
		REQUIRE_NE(inner_val_4, nullptr);
		EXPECT_EQ(*inner_val_4, 4.5);

		double const* inner_val_5 = tree/"An array"/2;
		EXPECT_EQ(inner_val_5, nullptr);
	}


	EXPECT_EQ((tree/4).is_null(), true);
	EXPECT_EQ(static_cast<int*>(tree/2), nullptr);

	EXPECT_EQ(tree.append<int>(345).is_null(), true);
	EXPECT_EQ(tree.non_recursive_size(), 4);

	try
	{
		EXPECT_EQ(tree.insert<int>("One", 35434).is_null(), true);
		abort();
	}
	catch(...)
	{}
	EXPECT_EQ(tree.non_recursive_size(), 4);

	tree.insert_or_assign<int>("One", 35434);
	EXPECT_EQ(tree.non_recursive_size(), 4);
	EXPECT_EQ(*static_cast<int*>(tree/"One"), 35434);

	tree.insert_or_assign<int>("New key", 454353);
	EXPECT_EQ(tree.non_recursive_size(), 5);
	EXPECT_EQ(*static_cast<int*>(tree/"New key"), 454353);
}