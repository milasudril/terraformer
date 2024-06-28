//@	{"target":{"name":"shared_any.test"}}

#include "./shared_any.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_shared_any_default_state)
{
	terraformer::shared_any object;
	EXPECT_EQ(object.get_if<int>(), nullptr);
	EXPECT_EQ(object.use_count(), 0);
	EXPECT_EQ(object, false);
}

TESTCASE(terraformer_shared_any_create_and_read_state)
{
	terraformer::shared_any object{std::type_identity<std::pair<int, int>>{}, 1234, 5678};
	EXPECT_EQ(object.get_if<int>(), nullptr);
	auto const ptr = object.get_if<std::pair<int, int>>();
	REQUIRE_NE(ptr, nullptr);
	EXPECT_EQ(ptr->first, 1234);
	EXPECT_EQ(ptr->second, 5678);
	EXPECT_EQ(object, true);
	EXPECT_EQ(object.use_count(), 1);
	int* const val_1 = object.get();
	EXPECT_EQ(val_1, nullptr);

	std::pair<int, int>* const val_2 = object.get();
	EXPECT_EQ(val_2, ptr);

	std::pair<int, int> const* val_3 = object.get();
	EXPECT_EQ(val_3, ptr);

// Should not compile
//std::pair<int, int>* val_bad = object.get_const();

	std::pair<int, int> const* val_4 = object.get_const();
	EXPECT_EQ(val_4, ptr);
}

TESTCASE(terraformer_shared_any_and_reset)
{
	terraformer::shared_any object{std::type_identity<std::pair<int, int>>{}, 1234, 5678};
	auto const ptr = object.get_if<std::pair<int, int>>();
	REQUIRE_NE(ptr, nullptr);
	EXPECT_EQ(ptr->first, 1234);
	EXPECT_EQ(ptr->second, 5678);
	EXPECT_EQ(object, true);
	EXPECT_EQ(object.use_count(), 1);
	EXPECT_NE(object, terraformer::shared_any{});

	object.reset();
	EXPECT_EQ(object.get_if<int>(), nullptr);
	EXPECT_EQ(object.use_count(), 0);
	EXPECT_EQ(object, false);
	EXPECT_EQ(object, terraformer::shared_any{});
}

TESTCASE(terraformer_shared_any_copy_construct)
{
	terraformer::shared_any object{std::type_identity<int>{}, 1234};
	auto const ptr_1 = object.get_if<int>();
	REQUIRE_NE(ptr_1, nullptr);
	EXPECT_EQ(*ptr_1, 1234);
	EXPECT_EQ(object, true);
	EXPECT_EQ(object.use_count(), 1);

	auto const other = object;
	auto const ptr_2 = other.get_if<int>();
	REQUIRE_NE(ptr_2, nullptr);
	EXPECT_EQ(ptr_2, ptr_1);
	EXPECT_EQ(*ptr_2, 1234);
	EXPECT_EQ(other, true);
	EXPECT_EQ(other.use_count(), 2);
	EXPECT_EQ(other.use_count(), 2);

	object.reset();
	EXPECT_EQ(object, terraformer::shared_any{});
	EXPECT_EQ(other.use_count(), 1);
	EXPECT_EQ(*other.get_if<int>(), 1234);
}

TESTCASE(terraformer_shared_any_move_construct)
{
	terraformer::shared_any object{std::type_identity<int>{}, 1234};
	auto const ptr_1 = object.get_if<int>();
	REQUIRE_NE(ptr_1, nullptr);
	EXPECT_EQ(*ptr_1, 1234);
	EXPECT_EQ(object, true);
	EXPECT_EQ(object.use_count(), 1);

	auto const other = std::move(object);
	EXPECT_EQ(object, terraformer::shared_any{});
	EXPECT_EQ(other.use_count(), 1);
	EXPECT_EQ(other, true);
	EXPECT_EQ(other.get_if<int>(), ptr_1);
}

TESTCASE(terraformer_shared_any_move_assign)
{
	terraformer::shared_any object_1{std::type_identity<int>{}, 1234};
	auto const ptr_1 = object_1.get_if<int>();
	REQUIRE_NE(ptr_1, nullptr);
	EXPECT_EQ(*ptr_1, 1234);
	EXPECT_EQ(object_1, true);
	EXPECT_EQ(object_1.use_count(), 1);

	terraformer::shared_any object_2{std::type_identity<double>{}, 8.5};
	auto const ptr_2 = object_2.get_if<double>();
	REQUIRE_NE(ptr_2, nullptr);
	EXPECT_EQ(*ptr_2, 8.5);
	EXPECT_EQ(object_2, true);
	EXPECT_EQ(object_2.use_count(), 1);

	object_1 = std::move(object_2);
	EXPECT_EQ(object_2, terraformer::shared_any{});
	auto const ptr_3 = object_1.get_if<double>();
	REQUIRE_NE(ptr_3, nullptr);
	EXPECT_EQ(ptr_3, ptr_2);
	EXPECT_EQ(*ptr_3, 8.5);
	EXPECT_EQ(object_1, true);
	EXPECT_EQ(object_1.use_count(), 1);
}

TESTCASE(terraformer_shared_any_copy_assign)
{
	terraformer::shared_any object_1{std::type_identity<int>{}, 1234};
	auto const ptr_1 = object_1.get_if<int>();
	REQUIRE_NE(ptr_1, nullptr);
	EXPECT_EQ(*ptr_1, 1234);
	EXPECT_EQ(object_1, true);
	EXPECT_EQ(object_1.use_count(), 1);

	terraformer::shared_any object_2{std::type_identity<double>{}, 8.5};
	auto const ptr_2 = object_2.get_if<double>();
	REQUIRE_NE(ptr_2, nullptr);
	EXPECT_EQ(*ptr_2, 8.5);
	EXPECT_EQ(object_2, true);
	EXPECT_EQ(object_2.use_count(), 1);

	object_1 = object_2;
	EXPECT_EQ(object_2, object_1);
	auto const ptr_3 = object_1.get_if<double>();
	REQUIRE_NE(ptr_3, nullptr);
	EXPECT_EQ(ptr_3, ptr_2);
	EXPECT_EQ(*ptr_3, 8.5);
	EXPECT_EQ(object_1, true);
	EXPECT_EQ(object_1.use_count(), 2);
	EXPECT_EQ(object_2.use_count(), 2);
}

namespace
{
	struct foo
	{
		foo()
		{ throw "Failed"; }
	};
}

TESTCASE(terraformer_shared_any_object_ctor_throws)
{
	try
	{
		terraformer::shared_any obj{std::type_identity<foo>{}};
		abort();
	}
	catch(...)
	{}
}

TESTCASE(terraformer_shared_const_any_create_and_access)
{
	terraformer::shared_const_any obj{std::type_identity<int>{}, 1};
	static_assert(!requires{
			{std::declval<terraformer::shared_const_any>().template get_if<int>() } -> std::same_as<int*>;
	});
	auto val = obj.get_if<int const>();
	static_assert(std::is_same_v<decltype(val), int const*>);
}