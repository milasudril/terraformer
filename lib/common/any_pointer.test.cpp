//@	{"target":{"name":"any_pointer.test"}}

#include "./any_pointer.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_any_pointer_make_pointer_to_const)
{
	auto x = 1234;
	static_assert(std::is_same_v<decltype(x), int>);

	auto y = terraformer::as_const(&x);
	EXPECT_EQ(y, &x);
	static_assert(std::is_same_v<decltype(y), int const*>);
}

TESTCASE(terraformer_any_pointer_default_state)
{
	{
		terraformer::any_pointer<false> ptr;
		EXPECT_EQ(ptr.type(), std::type_index{typeid(void)});
		EXPECT_EQ(ptr.pointer(), nullptr);
		EXPECT_EQ(ptr, false);
	}

	{
		terraformer::any_pointer<true> ptr;
		EXPECT_EQ(ptr.type(), std::type_index{typeid(void)});
		EXPECT_EQ(ptr.pointer(), nullptr);
		EXPECT_EQ(ptr, false);
	}
}

TESTCASE(terraformer_any_pointer_construct_forced_typeid)
{
	double x{1.0};
	terraformer::any_pointer ptr{&x, std::type_index{typeid(uint64_t)}};

	EXPECT_EQ(ptr, true);
	EXPECT_EQ(ptr.pointer(), &x);
	EXPECT_EQ(ptr.type(), std::type_index{typeid(uint64_t)});
}

TESTCASE(terraformer_any_pointer_construct_no_forced_type_id)
{
	double x{1.0};
	terraformer::any_pointer ptr{&x};

	EXPECT_EQ(ptr, true);
	EXPECT_EQ(ptr.pointer(), &x);
	EXPECT_EQ(ptr.type(), std::type_index{typeid(double)});

	{
		auto val_ptr = ptr.get_if<double>();
		static_assert(std::is_same_v<decltype(val_ptr), double*>);
		EXPECT_EQ(val_ptr, &x);
		EXPECT_EQ(*val_ptr, x);
	}

	{
		auto val_ptr = ptr.get_if<double const>();
		static_assert(std::is_same_v<decltype(val_ptr), double const*>);
		EXPECT_EQ(val_ptr, &x);
		EXPECT_EQ(*val_ptr, x);
	}

	{
		double* val_ptr = ptr;
		static_assert(std::is_same_v<decltype(val_ptr), double*>);
		EXPECT_EQ(val_ptr, &x);
		EXPECT_EQ(*val_ptr, x);
	}

	{
		double const* val_ptr = ptr;
		static_assert(std::is_same_v<decltype(val_ptr), double const*>);
		EXPECT_EQ(val_ptr, &x);
		EXPECT_EQ(*val_ptr, x);
	}

	{
		auto val_ptr = ptr.get_if<int>();
		static_assert(std::is_same_v<decltype(val_ptr), int*>);
		EXPECT_EQ(val_ptr, nullptr);
	}

	{
		auto val_ptr = ptr.get_if<int const>();
		static_assert(std::is_same_v<decltype(val_ptr), int const*>);
		EXPECT_EQ(val_ptr, nullptr);
	}

	{
		int* val_ptr = ptr;
		static_assert(std::is_same_v<decltype(val_ptr), int*>);
		EXPECT_EQ(val_ptr, nullptr);
	}

	{
		int const* val_ptr = ptr;
		static_assert(std::is_same_v<decltype(val_ptr), int const*>);
		EXPECT_EQ(val_ptr, nullptr);
	}
}

TESTCASE(terraformer_any_pointer_copy_ptr_to_ptr_to_const)
{
	double x{1.0};
	terraformer::any_pointer ptr{&x};

	EXPECT_EQ(ptr, true);
	EXPECT_EQ(ptr.pointer(), &x);
	EXPECT_EQ(ptr.type(), std::type_index{typeid(double)});

	terraformer::any_pointer<true> other{ptr};
	EXPECT_EQ(other, true);
	EXPECT_EQ(other.pointer(), &x);
	EXPECT_EQ(other.type(), std::type_index{typeid(double)});

	// Discards qualifiers double* val_ptr = other;
	double const* val_ptr = other;
	EXPECT_EQ(val_ptr, &x);
	EXPECT_EQ(*val_ptr, x);
}