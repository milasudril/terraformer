//@	{"target":{"name":"single_array.test"}}

#include "./single_array.hpp"

#include <cassert>
#include <testfwk/testfwk.hpp>
#include <testfwk/lifetime_checker.hpp>

namespace
{
	using default_constructible_type = testfwk::lifetime_checker<int>;

	class holder
	{
	public:
		explicit holder(int value):m_value{value}
		{}

		auto value() const
		{ return m_value; }

	private:
		int m_value;
	};

	using no_default_constructible_type = testfwk::lifetime_checker<holder>;
}

TESTCASE(terraformer_single_array_reseve_on_empty)
{
	terraformer::single_array<no_default_constructible_type> array;
	REQUIRE_EQ(array.size(), terraformer::array_size<no_default_constructible_type>{});
	REQUIRE_EQ(array.capacity(), terraformer::array_size<no_default_constructible_type>{});
	REQUIRE_EQ(array.begin(), array.end());

	array.reserve(terraformer::array_size<no_default_constructible_type>(5));

	EXPECT_EQ(array.size(), terraformer::array_size<no_default_constructible_type>{});
	EXPECT_EQ(array.capacity(), terraformer::array_size<no_default_constructible_type>{5});
	EXPECT_EQ(array.begin(), array.end());
}

TESTCASE(terraformer_single_array_push_back)
{
	terraformer::single_array<no_default_constructible_type> array;

	for(int k = 0; k != 16; ++k)
	{
		no_default_constructible_type::expect_ctor();
		if(static_cast<size_t>(k) == array.capacity().get())
		{ no_default_constructible_type::expect_move_ctor(array.capacity().get()); }
		no_default_constructible_type::expect_move_ctor();
		array.push_back(no_default_constructible_type{k});
	}

	for(auto k = array.first_element_index(); k != std::size(array); ++k)
	{ EXPECT_EQ(static_cast<size_t>(static_cast<holder>(array[k]).value()), k.get()); }

	EXPECT_EQ(std::size(array), terraformer::array_size<no_default_constructible_type>{16});

	{
		int k = 0;
		for(auto& item : array)
		{
			EXPECT_EQ(static_cast<holder>(item).value(), k);
			++k;
		}
		EXPECT_EQ(k, 16);
	}

	{
		auto span = static_cast<terraformer::span<no_default_constructible_type>>(array);
		for(auto k = array.first_element_index(); k != std::size(span); ++k)
		{ EXPECT_EQ(static_cast<size_t>(static_cast<holder>(array[k]).value()), k.get()); }
	}
}

TESTCASE(terraformer_single_array_resize_grow_and_shrink)
{
	terraformer::single_array<default_constructible_type> array;

	for(int k = 0; k != 4; ++k)
	{
		default_constructible_type::expect_ctor();
		default_constructible_type::expect_move_ctor();
		array.push_back(default_constructible_type{k});
	}

default_constructible_type::expect_move_ctor(4);
	default_constructible_type::expect_ctor(12);
	array.resize(terraformer::array_size<default_constructible_type>{16});

	EXPECT_EQ(array.capacity().get(), 16);
	EXPECT_EQ(array.size().get(), 16);
	for(auto k = array.first_element_index() + 4; k != std::size(array); ++k)
	{ EXPECT_EQ(static_cast<int>(array[k]), 0); }

	array.resize(terraformer::array_size<default_constructible_type>{8});
	EXPECT_EQ(array.size().get(), 8);
}

TESTCASE(terraformer_single_array_move)
{
	terraformer::single_array<no_default_constructible_type> array;

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

TESTCASE(terraformer_single_array_copy)
{
	terraformer::single_array<no_default_constructible_type> array;

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

TESTCASE(terraformer_single_array_move_assign)
{
	terraformer::single_array<no_default_constructible_type> array;

	no_default_constructible_type::expect_ctor(4);
	no_default_constructible_type::expect_move_ctor(4);
	array.push_back(1);
	array.push_back(2);
	array.push_back(3);
	array.push_back(4);

	terraformer::single_array<no_default_constructible_type> other;
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