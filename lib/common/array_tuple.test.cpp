//@	{"target":{"name":"array_tuple.test"}}

#include "./array_tuple.hpp"

#include "testfwk/testfwk.hpp"

#include <iterator>

namespace
{
	using array_tuple_non_copyable =
		terraformer::array_tuple<std::unique_ptr<int>, std::unique_ptr<double>>;

	using array_tulpe_not_trivally_copyable =
		terraformer::array_tuple<std::string, std::vector<size_t>>;

	using array_tuple_trivially_copyable = terraformer::array_tuple<int, double>;

	template<class T>
	auto generate(size_t);

	template<>
	auto generate<array_tuple_non_copyable::value_type>(size_t)
	{
		return array_tuple_non_copyable::value_type{};
	}

	constexpr char const* strings[]={"Hello", "World", "Lol"};

	template<>
	auto generate<array_tulpe_not_trivally_copyable::value_type>(size_t k)
	{
		return array_tulpe_not_trivally_copyable::value_type{
			std::string{strings[k%3]},
			std::vector{1*k, 2*k, 3*k}
		};
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
	void test_default_state()
	{
		T a;
		T b = T();

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
	void test_move_assign()
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
		EXPECT_EQ(b.capacity(), 16);

		EXPECT_EQ(a.capacity(), 0);
		EXPECT_EQ(std::size(a), 0);
		EXPECT_EQ(std::end(a) - std::begin(a), 0);

		a = std::move(b);
		EXPECT_EQ(std::size(a), 9);
		EXPECT_EQ(std::end(a) - std::begin(a), 9);
		EXPECT_EQ(a.capacity(), 16);

		EXPECT_EQ(b.capacity(), 0);
		EXPECT_EQ(std::size(b), 0);
		EXPECT_EQ(std::end(b) - std::begin(b), 0);

		auto i = std::begin(a);
		for(size_t k = 0; k != 9; ++k)
		{
			EXPECT_EQ(*i, generate<typename T::value_type>(k));
			++i;
		}
		EXPECT_EQ(i, std::end(a));
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

	template<class T>
	void test_copy_assign()
	{
		T a;
		for(size_t k = 0; k != 9; ++k)
		{ a.push_back(generate<typename T::value_type>(k)); }

		EXPECT_EQ(std::size(a), 9);
		EXPECT_EQ(std::end(a) - std::begin(a), 9);
		EXPECT_EQ(a.capacity(), 16);

		auto b = a;
		EXPECT_EQ(a, b);

		a = b;
		EXPECT_EQ(a, b);
	}

	template<class T>
	void test_iterator_ops()
	{
		T a;
		for(size_t k = 0; k != 9; ++k)
		{ a.push_back(generate<typename T::value_type>(k)); }

		EXPECT_EQ(std::size(a), 9);
		EXPECT_EQ(std::end(a) - std::begin(a), 9);
		EXPECT_EQ(a.capacity(), 16);

		auto i = std::begin(a);

		EXPECT_EQ(i[3], generate<typename T::value_type>(3));
		i += 3;
		EXPECT_EQ(*i, generate<typename T::value_type>(3));
		i -= 3;
		EXPECT_EQ(*i, generate<typename T::value_type>(0));
		EXPECT_EQ(i++, std::begin(a));
		EXPECT_EQ(*i, generate<typename T::value_type>(1));
		EXPECT_EQ(++i, std::begin(a) + 2);
		EXPECT_EQ(*i, generate<typename T::value_type>(2));
		EXPECT_EQ(--i, std::begin(a) + 1);
		EXPECT_EQ(*i, generate<typename T::value_type>(1));
		EXPECT_EQ(i--, std::begin(a) + 1);
		EXPECT_EQ(*i, generate<typename T::value_type>(0));
	}

	template<class T>
	void test_index_access()
	{
		T a;

		for(typename T::size_type k = 0; k != 9; ++k)
		{ a.push_back(generate<typename T::value_type>(k)); }

		for(typename T::size_type k = 0; k != 9; ++k)
		{
			EXPECT_EQ(a[k], generate<typename T::value_type>(k));
		}
	}

	template<class T>
	void test_assign()
	{
		T a;

		for(typename T::size_type k = 0; k != 9; ++k)
		{ a.push_back(generate<typename T::value_type>(k)); }

		for(typename T::size_type k = 0; k != 9; ++k)
		{
			EXPECT_EQ(a[k], generate<typename T::value_type>(k));
		}

		a.assign(3, generate<typename T::value_type>(50));
		// FIXME: EXPECT_NE(a[3], generate<typename T::value_type>(3));
		EXPECT_EQ(a[3], generate<typename T::value_type>(50));
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

TESTCASE(array_tuple_move_assign)
{
	test_move_assign<array_tuple_non_copyable>();
	test_move_assign<array_tulpe_not_trivally_copyable>();
	test_move_assign<array_tuple_trivially_copyable>();
}

TESTCASE(array_tuple_copy_construct)
{
	test_copy_construct<array_tulpe_not_trivally_copyable>();
	test_copy_construct<array_tuple_trivially_copyable>();
}

TESTCASE(array_tuple_copy_assign)
{
	test_copy_assign<array_tulpe_not_trivally_copyable>();
	test_copy_assign<array_tuple_trivially_copyable>();
}

TESTCASE(array_tuple_iterator_ops)
{
	test_iterator_ops<array_tuple_non_copyable>();
	test_iterator_ops<array_tulpe_not_trivally_copyable>();
	test_iterator_ops<array_tuple_trivially_copyable>();
}

TESTCASE(array_tuple_range_access)
{
	array_tuple_trivially_copyable a;
	for(size_t k = 0; k != 9; ++k)
	{ a.push_back(generate<array_tuple_trivially_copyable::value_type>(k)); }

	auto const r1 = get<0>(a);
	auto const r2 = get<1>(a);
	auto const r1_const = get<0>(std::as_const(a));
	auto const r2_const = get<1>(std::as_const(a));

	EXPECT_EQ(std::size(r1), std::size(r2));
	EXPECT_EQ(std::size(r1_const), std::size(r2_const));
	EXPECT_EQ(std::size(r1), std::size(a));
	EXPECT_EQ(std::size(r1_const), std::size(a));

	for(size_t k = 0; k != 9; ++k)
	{
		auto const expected_item = generate<array_tuple_trivially_copyable::value_type>(k);
		EXPECT_EQ(get<0>(expected_item), r1[k]);
		EXPECT_EQ(get<1>(expected_item), r2[k]);
		EXPECT_EQ(get<0>(expected_item), r1_const[k]);
		EXPECT_EQ(get<1>(expected_item), r2_const[k]);
	}
}

TESTCASE(array_tuple_index_access)
{
	test_index_access<array_tuple_non_copyable>();
	test_index_access<array_tulpe_not_trivally_copyable>();
	test_index_access<array_tuple_trivially_copyable>();
}

TESTCASE(array_tuple_assign)
{
	test_assign<array_tuple_non_copyable>();
	test_assign<array_tulpe_not_trivally_copyable>();
	test_assign<array_tuple_trivially_copyable>();
}