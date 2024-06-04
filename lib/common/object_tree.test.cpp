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
	EXPECT_EQ((vals/3).is_null(), true);

	auto lookup_val = std::as_const(vals)/0;
	static_assert(!std::is_convertible_v<int*, decltype(lookup_val)>);
	EXPECT_EQ(*static_cast<int const*>(lookup_val), 1);
	EXPECT_EQ((std::as_const(vals)/3).is_null(), true);

	vals.visit_elements([k = size_t{0}, &vals](auto index, auto object_ptr) mutable {
		static_assert(std::is_same_v<decltype(object_ptr), terraformer::object_pointer<false>>);
		EXPECT_EQ(object_ptr, vals/k);
		EXPECT_EQ(index.get(), k);
		++k;
	});

	std::as_const(vals).visit_elements([k = size_t{0}, &vals](auto index, auto object_ptr) mutable {
		static_assert(std::is_same_v<decltype(object_ptr), terraformer::object_pointer<true>>);
		EXPECT_EQ(object_ptr, vals/k);
		EXPECT_EQ(index.get(), k);
		++k;
	});
}

TESTCASE(terraformer_object_dict)
{
	// Initial state
	terraformer::object_dict vals;
	EXPECT_EQ(vals.empty(), true);

	// Insert
	vals.insert<int>("One", 1)
		.insert<double>("Pi", std::numbers::pi_v<double>)
		.insert<std::string>("Foo", "This is a longer string lol");

	EXPECT_EQ(vals.empty(), false);
	EXPECT_EQ(vals.size(), 3);
	EXPECT_EQ(*static_cast<int*>(vals/"One"), 1);
	EXPECT_EQ(*static_cast<double*>(vals/"Pi"), std::numbers::pi_v<double>);
	EXPECT_EQ(*static_cast<std::string*>(vals/"Foo"), "This is a longer string lol");
	EXPECT_EQ(static_cast<double*>(vals/"One"), nullptr);
	EXPECT_EQ((vals/"Bajs").is_null(), true);

	auto lookup_val = std::as_const(vals)/"One";
	static_assert(!std::is_convertible_v<int*, decltype(lookup_val)>);
	EXPECT_EQ(*static_cast<int const*>(lookup_val), 1);
	EXPECT_EQ((std::as_const(vals)/"Bajs").is_null(), true);

	// Insert existing value
	try
	{
		vals.insert<int>("One", 2);
		abort();
	}
	catch(...)
	{}

	vals.insert_or_assign<int>("One", 2);
	EXPECT_EQ(*static_cast<int*>(vals/"One"), 2);

	vals.insert_or_assign<double>("e", std::numbers::e_v<double>);
	EXPECT_EQ(*static_cast<double*>(vals/"e"), std::numbers::e_v<double>);

	size_t iter_count = 0;
	vals.visit_elements([&iter_count, &vals](auto const& key, auto object_ptr) mutable {
		static_assert(std::is_same_v<decltype(object_ptr), terraformer::object_pointer<false>>);
		EXPECT_EQ(object_ptr, vals/key);
		++iter_count;
	});
	EXPECT_EQ(iter_count, std::size(vals));

	iter_count = 0;
	std::as_const(vals).visit_elements([&iter_count, &vals](auto const& key, auto object_ptr) mutable {
		static_assert(std::is_same_v<decltype(object_ptr), terraformer::object_pointer<true>>);
		EXPECT_EQ(object_ptr, vals/key);
		++iter_count;
	});
	EXPECT_EQ(iter_count, std::size(vals));
}

namespace
{
	template<bool IsConst>
	struct item_visitor
	{
		item_visitor()
		{ puts("{"); }

		~item_visitor()
		{ puts("}"); }

		void operator()(std::string const& key, terraformer::object_pointer<IsConst> object)
		{
			printf("%s: ", key.c_str());
			object.visit_elements(item_visitor<IsConst>{});
		}

		void operator()(terraformer::object_array::index_type key, terraformer::object_pointer<IsConst> object)
		{
			printf("%zu: ", key.get());
			object.visit_elements(item_visitor<IsConst>{});
		}

		void operator()(terraformer::object_pointer<IsConst> obj)
		{
			printf("%p\n", obj.pointer().pointer());
		}
	};
}

TESTCASE(terraformer_object_tree_object_pointer)
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
		EXPECT_EQ((vals/"One").size(), 1);
	}

	{
		double const* val = vals/"Two";
		REQUIRE_NE(val, nullptr);
		EXPECT_EQ(*val, 2.5);
		EXPECT_EQ((vals/"Two").size(), 1);
	}

	{
		std::string const* val = vals/"Three";
		REQUIRE_NE(val, nullptr);
		EXPECT_EQ(*val, "Hello, World");
		EXPECT_EQ((vals/"Three").size(), 1);
	}

	EXPECT_EQ((vals/"Non-existing value").size(), 0);

	{
		terraformer::object_array* val = vals/"An array";
		REQUIRE_NE(val, nullptr);
		REQUIRE_NE(std::size(*val), terraformer::object_array::size_type{0});
		EXPECT_EQ(std::size(*val).get(), (vals/"An array").size());
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
		EXPECT_EQ(std::size(*val), (vals/"A dictionary").size());
		int const* inner_val = (*val)/"Foo";
		REQUIRE_NE(inner_val, nullptr);
		EXPECT_EQ(*inner_val, 14);
		EXPECT_EQ(*static_cast<int*>(vals/"A dictionary"/"Foo"), 14)
		EXPECT_EQ((vals/"A dictionary"/0).is_null(), true);
	}

	{
		EXPECT_EQ((vals/"A dictionary").append<std::string>("Fail").is_null(), true);
		EXPECT_EQ((vals/"A dictionary").size(), 1);
		EXPECT_EQ((vals/"A dictionary").insert<std::string>("value", "Success").is_null(), false);
		EXPECT_EQ(*static_cast<std::string*>(vals/"A dictionary"/"value"), "Success");
		EXPECT_EQ((vals/"A dictionary").size(), 2);
		try
		{
			(vals/"A dictionary").insert<std::string>("value", "Success");
			abort();
		}
		catch(...)
		{}
		EXPECT_EQ((vals/"A dictionary").size(), 2);
		EXPECT_EQ((std::as_const(vals)/"A dictionary").size(), 2);
		EXPECT_EQ((vals/"A dictionary").insert_or_assign<std::string>("value", "A new value").is_null(), false);
		EXPECT_EQ(*static_cast<std::string*>(vals/"A dictionary"/"value"), "A new value");
	}

	{
		EXPECT_EQ((vals/"An array").insert<std::string>("value", "fail").is_null(), true);
		EXPECT_EQ((vals/"An array").size(), 2);
		EXPECT_EQ((vals/"An array").insert_or_assign<std::string>("value", "fail").is_null(), true);
		EXPECT_EQ((vals/"An array").size(), 2);
		EXPECT_EQ((vals/"An array").append<std::string>("a value").is_null(), false);
		EXPECT_EQ((vals/"An array").size(), 3);
		EXPECT_EQ((std::as_const(vals)/"An array").size(), 3);
		EXPECT_EQ(*static_cast<std::string*>(vals/"An array"/2), "a value");
	}

	{
		EXPECT_EQ((vals/"One").size(), 1);
		EXPECT_EQ((vals/"One").append<int>(1).is_null(), true);
		EXPECT_EQ((vals/"One").size(), 1);
		EXPECT_EQ((vals/"One").insert<int>("Other value", 1).is_null(), true);
		EXPECT_EQ((vals/"One").size(), 1);
		EXPECT_EQ((vals/"One").insert_or_assign<int>("Other value", 1).is_null(), true);
		EXPECT_EQ((vals/"One").size(), 1);
	}

	puts("=====================");
	vals.visit_elements(item_visitor<false>{});
	puts("=====================");
	std::as_const(vals).visit_elements(item_visitor<true>{});
	puts("=====================");
}