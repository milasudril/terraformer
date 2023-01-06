//@	{"target":{"name":"array_tuple.test"}}

#include "./array_tuple.hpp"

#include "testfwk/testfwk.hpp"

#include <iterator>
#include <cxxabi.h>
#include <typeinfo>

namespace
{
	using array_tuple_non_copyable =
		terraformer::array_tuple<std::unique_ptr<int>, std::unique_ptr<double>>;

	using array_tulpe_not_trivally_copyable =
		terraformer::array_tuple<std::string, std::vector<int>>;

	using array_tuple_trivially_copyable = terraformer::array_tuple<int, double>;

	template<class T>
	auto generate(size_t);

	template<>
	auto generate<array_tuple_non_copyable::value_type>(size_t)
	{
		return array_tuple_non_copyable::value_type{};
	}

	template<>
	auto generate<array_tulpe_not_trivally_copyable::value_type>(size_t)
	{
		return array_tulpe_not_trivally_copyable::value_type{};
	}

	template<>
	auto generate<array_tuple_trivially_copyable::value_type>(size_t k)
	{
		return array_tuple_trivially_copyable::value_type{
			static_cast<int>(k),
			0.5*static_cast<double>(k)
		};
	}
}

static_assert(std::random_access_iterator<array_tuple_trivially_copyable::const_iterator>);

static_assert(std::random_access_iterator<array_tuple_non_copyable::const_iterator>);

static_assert(std::random_access_iterator<array_tulpe_not_trivally_copyable::const_iterator>);

namespace
{
	template<class T>
	void print_typename()
	{
		auto str = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
		if(str == nullptr)
		{ return; }

		puts(str);
	}

	template<class T>
	void test_default_state()
	{
		T a;
		T b = T();

		print_typename<typename T::value_type>();
		EXPECT_EQ(a.empty(), true);
		EXPECT_EQ(b.empty(), true);
	}

	template<class T>
	void test_push_back()
	{
		T a;

		for(size_t k = 0; k != 9; ++k)
		{ a.push_back(generate<typename T::value_type>(k)); }

		EXPECT_EQ(std::size(a), 9);
		EXPECT_EQ(std::end(a) - std::begin(a), 9);
		EXPECT_EQ(a.capacity(), 16);

		auto i = std::begin(a);
		for(size_t k = 0; k != 9; ++k)
		{
			EXPECT_EQ(*i, generate<typename T::value_type>(k));
			++i;
		}
		EXPECT_EQ(i, std::end(a));
	}

	template<class T>
	void test_move_construct()
	{
		T a;
		for(size_t k = 0; k != 9; ++k)
		{ a.push_back(generate<typename T::value_type>(k)); }

		EXPECT_EQ(std::size(a), 9);
		EXPECT_EQ(std::end(a) - std::begin(a), 9);
		EXPECT_EQ(a.capacity(), 16);

		auto b = std::move(a);
		EXPECT_EQ(std::size(b), 9);
		EXPECT_EQ(std::end(b) - std::begin(b), 9);
		EXPECT_EQ(a.capacity(), 0);
		EXPECT_EQ(std::size(a), 0);
		EXPECT_EQ(std::end(a) - std::begin(a), 0);
		EXPECT_EQ(a.capacity(), 0);

		auto i = std::begin(b);
		for(size_t k = 0; k != 9; ++k)
		{
			EXPECT_EQ(*i, generate<typename T::value_type>(k));
			++i;
		}
		EXPECT_EQ(i, std::end(b));
	}

	template<class T>
	void test_copy_construct()
	{
		T a;
		for(size_t k = 0; k != 9; ++k)
		{ a.push_back(generate<typename T::value_type>(k)); }

		EXPECT_EQ(std::size(a), 9);
		EXPECT_EQ(std::end(a) - std::begin(a), 9);
		EXPECT_EQ(a.capacity(), 16);

		auto b = a;
		EXPECT_EQ(a, b);
	}
}

TESTCASE(array_tuple_default_state)
{
	test_default_state<array_tuple_non_copyable>();
	test_default_state<array_tulpe_not_trivally_copyable>();
	test_default_state<array_tuple_trivially_copyable>();
}

TESTCASE(array_tulpe_push_back)
{
	test_push_back<array_tuple_non_copyable>();
	test_push_back<array_tulpe_not_trivally_copyable>();
	test_push_back<array_tuple_trivially_copyable>();
}

TESTCASE(array_tuple_move_construct)
{
	test_move_construct<array_tuple_non_copyable>();
	test_move_construct<array_tulpe_not_trivally_copyable>();
	test_move_construct<array_tuple_trivially_copyable>();
}

TESTCASE(array_tuple_copy_construct)
{
	test_copy_construct<array_tulpe_not_trivally_copyable>();
	test_copy_construct<array_tuple_trivially_copyable>();
}