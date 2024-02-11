//@	{"target":{"name":"multi_array.test"}}

#include "./multi_array.hpp"

#include <cassert>
#include <testfwk/testfwk.hpp>
#include <testfwk/lifetime_checker.hpp>

namespace
{
	template<class T>
	using default_constructible_type = testfwk::lifetime_checker<T>;

	template<class T>
	class holder
	{
	public:
		explicit holder(T value):m_value{value}
		{}

		auto value() const
		{ return m_value; }

	private:
		T m_value;
	};

	template<class T>
	using no_default_constructible_type = testfwk::lifetime_checker<holder<T>>;
}

TESTCASE(terraformer_multi_array_generate_mem_blocks)
{
	auto blocks = generate_mem_blocks(terraformer::array_size<terraformer::multi_array_tag<int, double>>{12});
	EXPECT_EQ(std::size(blocks), 2);
	EXPECT_NE(blocks[0].get(), nullptr);
	EXPECT_NE(blocks[1].get(), nullptr);
}

TESTCASE(terraformer_mulite_array_uninitialized_copy)
{
	using size_type = terraformer::array_size<
		terraformer::multi_array_tag<
			default_constructible_type<int>,
			default_constructible_type<double>
		>
	>;

	auto blocks = generate_mem_blocks(size_type{3});

	EXPECT_EQ(std::size(blocks), 2);
	default_constructible_type<int>::expect_ctor(3);
	std::uninitialized_value_construct_n(blocks[0].interpret_as<default_constructible_type<int>>(), 3);

	default_constructible_type<double>::expect_ctor(3);
	std::uninitialized_value_construct_n(blocks[1].interpret_as<default_constructible_type<double>>(), 3);

	auto new_blocks = generate_mem_blocks(size_type{3});
	default_constructible_type<int>::expect_copy_ctor(3);
	default_constructible_type<double>::expect_copy_ctor(3);
	uninitialized_copy(blocks, new_blocks, size_type{3});

	std::destroy_n(blocks[0].interpret_as<default_constructible_type<int>>(), 3);
	std::destroy_n(blocks[1].interpret_as<default_constructible_type<double>>(), 3);

	std::destroy_n(new_blocks[0].interpret_as<default_constructible_type<int>>(), 3);
	std::destroy_n(new_blocks[1].interpret_as<default_constructible_type<double>>(), 3);
}

TESTCASE(terraformer_multi_array_reseve_on_empty)
{
	terraformer::multi_array<
		no_default_constructible_type<int>,
		no_default_constructible_type<double>
	> array;
	using size_type = decltype(array)::size_type;

	{
		REQUIRE_EQ(array.size(), size_type{});
		REQUIRE_EQ(array.capacity(), size_type{});
		auto const first_array = array.get<0>();
		REQUIRE_EQ(std::size(first_array).get(), 0);
		auto const second_array = array.get<1>();
		REQUIRE_EQ(std::size(second_array).get(), 0);
	}

	array.reserve(size_type{5});

	{
		EXPECT_EQ(array.size(), size_type{});
		EXPECT_EQ(array.capacity(), size_type{5});
		auto const first_array = array.get<0>();
		EXPECT_EQ(std::size(first_array).get(), 0);
		auto const second_array = array.get<1>();
		EXPECT_EQ(std::size(second_array).get(), 0);
	}
}

TESTCASE(terraformer_multi_array_push_back)
{
	terraformer::multi_array<
		no_default_constructible_type<int>,
		no_default_constructible_type<double>
	> array;

	for(int k = 0; k != 16; ++k)
	{
		if(static_cast<size_t>(k) == array.capacity().get())
		{
			no_default_constructible_type<int>::expect_move_ctor(array.capacity().get());
			no_default_constructible_type<double>::expect_move_ctor(array.capacity().get());
		}

		no_default_constructible_type<int>::expect_ctor();
		no_default_constructible_type<double>::expect_ctor();
		no_default_constructible_type<int>::expect_move_ctor();
		no_default_constructible_type<double>::expect_move_ctor();

		array.push_back(
			no_default_constructible_type<int>{k},
			no_default_constructible_type<double>{0.5*static_cast<double>(k)}
		);
	}

	auto const first_array = array.get<0>();
	auto const second_array = array.get<1>();

	for(auto k = array.first_element_index(); k != std::size(array); ++k)
	{
		EXPECT_EQ(static_cast<size_t>(static_cast<holder<int>>(first_array[k]).value()), k.get());
		EXPECT_EQ(static_cast<holder<double>>(second_array[k]).value(), 0.5*static_cast<double>(k.get()));
	}

	using size_type = terraformer::multi_array<
		no_default_constructible_type<int>,
		no_default_constructible_type<double>
	>::size_type;
	EXPECT_EQ(std::size(array), size_type{16});
}

TESTCASE(terraformer_multi_array_resize_grow_and_shrink)
{
	terraformer::multi_array<
		default_constructible_type<int>,
		default_constructible_type<double>
	> array;

	for(int k = 0; k != 4; ++k)
	{
		default_constructible_type<int>::expect_ctor();
		default_constructible_type<int>::expect_move_ctor();
		default_constructible_type<double>::expect_ctor();
		default_constructible_type<double>::expect_move_ctor();
		array.push_back(
			default_constructible_type<int>{k},
			default_constructible_type<double>{static_cast<double>(k)/2.0}
		);
	}

	default_constructible_type<int>::expect_move_ctor(4);
	default_constructible_type<double>::expect_move_ctor(4);
	default_constructible_type<int>::expect_ctor(12);
	default_constructible_type<double>::expect_ctor(12);

	using size_type = decltype(array)::size_type;
	array.resize(size_type{16});

	EXPECT_EQ(array.capacity().get(), 16);
	EXPECT_EQ(array.size().get(), 16);
	auto const first_array = array.get<0>();
	auto const second_array = array.get<1>();
	for(auto k = array.first_element_index() + 4; k != std::size(array); ++k)
	{
		EXPECT_EQ(static_cast<size_t>(static_cast<holder<int>>(first_array[k]).value()), 0);
		EXPECT_EQ(static_cast<holder<double>>(second_array[k]).value(), 0.0);
	}

	array.resize(size_type{8});
	EXPECT_EQ(array.size().get(), 8);
}
#if 0

TESTCASE(terraformer_multi_array_move)
{
	terraformer::multi_array<no_default_constructible_type> array;

	no_default_constructible_type::expect_ctor(4);
	no_default_constructible_type::expect_move_ctor(4);
	array.push_back(1);
	array.push_back(2);
	array.push_back(3);
	array.push_back(4);

	auto const old_ptr = std::data(array);
	auto other = std::move(array);

	EXPECT_EQ(std::size(array).get(), 0);
	EXPECT_EQ(std::size(other).get(), 4);
	EXPECT_EQ(std::data(other), old_ptr);
	EXPECT_EQ(std::data(array), nullptr);
}

TESTCASE(terraformer_multi_array_copy)
{
	terraformer::multi_array<no_default_constructible_type> array;

	no_default_constructible_type::expect_ctor(4);
	no_default_constructible_type::expect_move_ctor(4);
	array.push_back(1);
	array.push_back(2);
	array.push_back(3);
	array.push_back(4);

	auto const old_ptr = std::data(array);

	no_default_constructible_type::expect_copy_ctor(4);
	auto const other = array;

	EXPECT_EQ(std::size(array).get(), 4);
	EXPECT_EQ(std::size(other).get(), 4);
	EXPECT_NE(std::data(other), old_ptr);
	EXPECT_NE(std::data(other), nullptr);
	EXPECT_EQ(std::data(array), old_ptr);
}

TESTCASE(terraformer_multi_array_move_assign)
{
		terraformer::multi_array<no_default_constructible_type> array;

		no_default_constructible_type::expect_ctor(4);
		no_default_constructible_type::expect_move_ctor(4);
		array.push_back(1);
		array.push_back(2);
		array.push_back(3);
		array.push_back(4);

		terraformer::multi_array<no_default_constructible_type> other;
		no_default_constructible_type::expect_ctor(3);
		no_default_constructible_type::expect_move_ctor(3);
		other.push_back(5);
		other.push_back(6);
		other.push_back(7);

		auto new_ptr = std::data(other);

		array = std::move(other);
		EXPECT_EQ(std::data(array), new_ptr);
		EXPECT_EQ(std::data(other), nullptr);
		EXPECT_EQ(std::size(array).get(), 3);
		EXPECT_EQ(std::size(other).get(), 0);
}
#endif
