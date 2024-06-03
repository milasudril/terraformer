//@	{"target":{"name":"object_tree.test"}}

#include "./object_tree.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_object_array)
{
	terraformer::object_array vals;
	EXPECT_EQ(vals.empty(), true);

	vals.append<int>(1)
		.append<double>(2.5)
		.append<std::string>("Hello, World");

	EXPECT_EQ(vals.empty(), false);
	REQUIRE_EQ(vals.size(), terraformer::object_array::size_type{3});
	EXPECT_EQ(*static_cast<int*>(vals/0), 1);
	EXPECT_EQ(*static_cast<double*>(vals/1), 2.5);
	EXPECT_EQ(*static_cast<std::string*>(vals/2), "Hello, World");
	EXPECT_EQ(static_cast<double*>(vals/0), nullptr);
	EXPECT_EQ(static_cast<double*>(vals/3), nullptr);

	auto lookup_val = std::as_const(vals)/0;

	static_assert(!std::is_convertible_v<int*, decltype(lookup_val)>);
	EXPECT_EQ(*static_cast<int const*>(lookup_val), 1);
}

#if 0

TESTCASE(terraformer_object_tree_append)
{
	terraformer::object_array vals;
	EXPECT_EQ(vals.empty(), true);
	vals.append<int>(1)
		.append<double>(2.5)
		.append<std::string>("Hello, World")
		.append<terraformer::object_array>(
			terraformer::object_array{}
				.append<int>(3)
		);
	EXPECT_EQ(vals.empty(), false);

	REQUIRE_EQ(vals.size(), terraformer::object_array::size_type{4});

	{
		int const* val = vals/0;
		REQUIRE_NE(val, nullptr);
		EXPECT_EQ(*val, 1);
	}

	{
		double const* val = vals/1;
		REQUIRE_NE(val, nullptr);
		EXPECT_EQ(*val, 2.5);
	}

	{
		std::string const* val = vals/2;
		REQUIRE_NE(val, nullptr);
		EXPECT_EQ(*val, "Hello, World");
	}

	{
		terraformer::object_array* val = vals/3;
		REQUIRE_NE(val, nullptr);
		REQUIRE_NE(std::size(*val), terraformer::object_array::size_type{0});

		int const* inner_val = (*val)/0;
		REQUIRE_NE(inner_val, nullptr);
		EXPECT_EQ(*inner_val, 3);

		int const* inner_val_2 = vals/3/0;
		REQUIRE_NE(inner_val_2, nullptr);
		EXPECT_EQ(inner_val_2, inner_val);

		int const* inner_val_3 = vals/3/1;
		EXPECT_EQ(inner_val_3, nullptr);
	}

	EXPECT_EQ((vals/4).is_null(), true);
	EXPECT_EQ(static_cast<int*>(vals/2), nullptr);
}

TESTCASE(terraformer_object_tree_insert)
{
	terraformer::object_dict vals;
	EXPECT_EQ(vals.empty(), true);
	vals.insert<int>("One", 1)
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
		)
		.insert<terraformer::object_dict>(
			"A dictionary",
			terraformer::object_dict{}.insert<int>("Foo", 14)
		);

	REQUIRE_EQ(vals.size(), 5);
	{
		int const* val = vals/"One";
		REQUIRE_NE(val, nullptr);
		EXPECT_EQ(*val, 1);
	}

	{
		double const* val = vals/"Two";
		REQUIRE_NE(val, nullptr);
		EXPECT_EQ(*val, 2.5);
	}

	{
		std::string const* val = vals/"Three";
		REQUIRE_NE(val, nullptr);
		EXPECT_EQ(*val, "Hello, World");
	}

	{
		terraformer::object_array* val = vals/"An array";
		REQUIRE_NE(val, nullptr);
		REQUIRE_NE(std::size(*val), terraformer::object_array::size_type{0});
		int const* inner_val = (*val)/0;
		REQUIRE_NE(inner_val, nullptr);
		EXPECT_EQ(*inner_val, 3);
		int const* inner_val_2 = vals/"An array"/0;
		REQUIRE_NE(inner_val_2, nullptr);
		EXPECT_EQ(inner_val_2, inner_val);

		double const* inner_val_3 = vals/"An array"/1/"A value";
		REQUIRE_NE(inner_val_3, nullptr);
		EXPECT_EQ(*inner_val_3, 6.5);

		double const* inner_val_4 = vals/"An array"/1/"Another value";
		REQUIRE_NE(inner_val_4, nullptr);
		EXPECT_EQ(*inner_val_4, 4.5);

		double const* inner_val_5 = vals/"An array"/2;
		EXPECT_EQ(inner_val_5, nullptr);

		EXPECT_EQ((vals/"An array"/"Key in array").is_null(), true);
	}

	{
		terraformer::object_dict* val = vals/"A dictionary";
		REQUIRE_NE(val, nullptr);
		int const* inner_val = (*val)/"Foo";
		REQUIRE_NE(inner_val, nullptr);
		EXPECT_EQ(*inner_val, 14);
		EXPECT_EQ(*static_cast<int*>(vals/"A dictionary"/"Foo"), 14)
		EXPECT_EQ((vals/"A dictionary"/0).is_null(), true);
	}

	try
	{
		vals.insert<int>("One", 35434);
		abort();
	}
	catch(...)
	{}
	EXPECT_EQ(vals.size(), 5);


	vals.insert_or_assign<int>("One", 35434);
	EXPECT_EQ(vals.size(), 5);
	EXPECT_EQ(*static_cast<int*>(vals/"One"), 35434);

	vals.insert_or_assign<int>("New key", 454353);
	EXPECT_EQ(vals.size(), 6);
	EXPECT_EQ(*static_cast<int*>(vals/"New key"), 454353);
}
#endif