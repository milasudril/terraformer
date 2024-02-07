//@	{"target":{"name":"single_array.test"}}

#include "./single_array.hpp"

#include <cassert>
#include <testfwk/testfwk.hpp>

namespace
{
	class no_default_constructible_type
	{
	public:
		static size_t expected_init;
		static size_t expected_move_ctor;
		static size_t expected_dtor;
		static size_t expected_copy_ctor;

		no_default_constructible_type(int value):m_value{value}
		{
			if(expected_init == 0)
			{	throw std::runtime_error{"Unexpected init"}; }

			--expected_init;
			++expected_dtor;
		}

		no_default_constructible_type(no_default_constructible_type const& other):m_value{other.m_value}
		{
			if(expected_copy_ctor == 0)
			{ throw std::runtime_error{"Unexpected copy ctor"}; }
			--expected_copy_ctor;
			++expected_dtor;
		}

		no_default_constructible_type(no_default_constructible_type&& other): m_value{other.m_value}
		{
			if(expected_move_ctor == 0)
			{ throw std::runtime_error{"Unexpected move ctor"}; }

			--expected_move_ctor;
			++expected_dtor;
		}

		~no_default_constructible_type()
		{
			assert(expected_dtor != 0);
			--expected_dtor;
		}

		auto value() const
		{ return m_value; }
	private:
		int m_value;
	};

	constinit size_t no_default_constructible_type::expected_init = 0;
	constinit size_t no_default_constructible_type::expected_move_ctor = 0;
	constinit size_t no_default_constructible_type::expected_dtor = 0;
	constinit size_t no_default_constructible_type::expected_copy_ctor = 0;


	class default_constructible_type
	{
	public:
		static size_t expected_init;
		static size_t expected_move_ctor;
		static size_t expected_dtor;
		static size_t expected_default_ctor;

		default_constructible_type(): m_value{123}
		{
			if(expected_default_ctor == 0)
			{ throw std::runtime_error{"Unexpected default constructor"}; }

			--expected_default_ctor;
			++expected_dtor;
		}

		default_constructible_type(int value):m_value{value}
		{
			if(expected_init == 0)
			{	throw std::runtime_error{"Unexpected init"}; }

			--expected_init;
			++expected_dtor;
		}

		default_constructible_type(default_constructible_type&& other): m_value{other.m_value}
		{
			if(expected_move_ctor == 0)
			{ throw std::runtime_error{"Unexpected move ctor"}; }

			--expected_move_ctor;
			++expected_dtor;
		}

		~default_constructible_type()
		{
			assert(expected_dtor != 0);
			--expected_dtor;
		}

		auto value() const
		{ return m_value; }

	private:
		int m_value;
	};

	constinit size_t default_constructible_type::expected_default_ctor = 0;
	constinit size_t default_constructible_type::expected_init = 0;
	constinit size_t default_constructible_type::expected_move_ctor = 0;
	constinit size_t default_constructible_type::expected_dtor = 0;
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
	{
		terraformer::single_array<no_default_constructible_type> array;

		for(int k = 0; k != 16; ++k)
		{
			++no_default_constructible_type::expected_init;
			++no_default_constructible_type::expected_move_ctor;

			if(static_cast<size_t>(k) == array.capacity().get())
			{ no_default_constructible_type::expected_move_ctor += array.capacity().get(); }

			array.push_back(no_default_constructible_type{k});
		}

		EXPECT_EQ(no_default_constructible_type::expected_init, 0);
		EXPECT_EQ(no_default_constructible_type::expected_move_ctor, 0);

		for(auto k = array.first_element_index(); k != std::size(array); ++k)
		{ EXPECT_EQ(static_cast<size_t>(array[k].value()), k.get()); }

		EXPECT_EQ(std::size(array), terraformer::array_size<no_default_constructible_type>{16});

		{
			int k = 0;
			for(auto& item : array)
			{
				EXPECT_EQ(item.value(), k);
				++k;
			}
			EXPECT_EQ(k, 16);
		}

		{
			auto span = static_cast<terraformer::span<no_default_constructible_type>>(array);
			for(auto k = array.first_element_index(); k != std::size(span); ++k)
			{ EXPECT_EQ(static_cast<size_t>(array[k].value()), k.get()); }
		}
	}

	EXPECT_EQ(no_default_constructible_type::expected_dtor, 0);
}

TESTCASE(terraformer_single_array_resize_grow_and_shrink)
{
	{
		terraformer::single_array<default_constructible_type> array;

		for(int k = 0; k != 4; ++k)
		{
			++default_constructible_type::expected_init;
			++default_constructible_type::expected_move_ctor;

			if(static_cast<size_t>(k) == array.capacity().get())
			{ default_constructible_type::expected_move_ctor += array.capacity().get(); }

			array.push_back(default_constructible_type{k});
		}

		default_constructible_type::expected_default_ctor += 12;
		default_constructible_type::expected_move_ctor += 4;
		array.resize(terraformer::array_size<default_constructible_type>{16});

		EXPECT_EQ(default_constructible_type::expected_default_ctor, 0)
		EXPECT_EQ(default_constructible_type::expected_move_ctor, 0)

		EXPECT_EQ(array.capacity().get(), 16);
		EXPECT_EQ(array.size().get(), 16);
		for(auto k = array.first_element_index() + 4; k != std::size(array); ++k)
		{ EXPECT_EQ(array[k].value(), 123); }


		array.resize(terraformer::array_size<default_constructible_type>{8});
		EXPECT_EQ(array.size().get(), 8);
		EXPECT_EQ(default_constructible_type::expected_dtor, 8);
	}

	EXPECT_EQ(default_constructible_type::expected_dtor, 0);
}

TESTCASE(terraformer_single_array_move)
{
	{
		terraformer::single_array<no_default_constructible_type> array;

		no_default_constructible_type::expected_init += 4;
		no_default_constructible_type::expected_move_ctor += 4;
		array.push_back(1);
		array.push_back(2);
		array.push_back(3);
		array.push_back(4);

		EXPECT_EQ(no_default_constructible_type::expected_init, 0);
		EXPECT_EQ(no_default_constructible_type::expected_move_ctor, 0);

		auto const old_ptr = std::data(array);
		auto other = std::move(array);

		EXPECT_EQ(std::size(array).get(), 0);
		EXPECT_EQ(std::size(other).get(), 4);
		EXPECT_EQ(std::data(other), old_ptr);
		EXPECT_EQ(std::data(array), nullptr);
	}
	EXPECT_EQ(no_default_constructible_type::expected_dtor, 0);
}

TESTCASE(terraformer_single_array_copy)
{
	{
		terraformer::single_array<no_default_constructible_type> array;

		no_default_constructible_type::expected_init += 4;
		no_default_constructible_type::expected_move_ctor += 4;
		array.push_back(1);
		array.push_back(2);
		array.push_back(3);
		array.push_back(4);

		EXPECT_EQ(no_default_constructible_type::expected_init, 0);
		EXPECT_EQ(no_default_constructible_type::expected_move_ctor, 0);

		auto const old_ptr = std::data(array);

		no_default_constructible_type::expected_copy_ctor += 4;
		auto const other = array;
		EXPECT_EQ(no_default_constructible_type::expected_copy_ctor, 0);

		EXPECT_EQ(std::size(array).get(), 4);
		EXPECT_EQ(std::size(other).get(), 4);
		EXPECT_NE(std::data(other), old_ptr);
		EXPECT_NE(std::data(other), nullptr);
		EXPECT_EQ(std::data(array), old_ptr);
	}
	EXPECT_EQ(no_default_constructible_type::expected_dtor, 0);
}

TESTCASE(terraformer_single_array_move_assign)
{
	{
		terraformer::single_array<no_default_constructible_type> array;

		no_default_constructible_type::expected_init += 4;
		no_default_constructible_type::expected_move_ctor += 4;
		array.push_back(1);
		array.push_back(2);
		array.push_back(3);
		array.push_back(4);
		EXPECT_EQ(no_default_constructible_type::expected_init, 0);
		EXPECT_EQ(no_default_constructible_type::expected_move_ctor, 0);

		terraformer::single_array<no_default_constructible_type> other;
		no_default_constructible_type::expected_init += 3;
		no_default_constructible_type::expected_move_ctor += 3;
		other.push_back(5);
		other.push_back(6);
		other.push_back(7);
		EXPECT_EQ(no_default_constructible_type::expected_init, 0);
		EXPECT_EQ(no_default_constructible_type::expected_move_ctor, 0);

		auto new_ptr = std::data(other);

		array = std::move(other);
		EXPECT_EQ(std::data(array), new_ptr);
		EXPECT_EQ(std::data(other), nullptr);
		EXPECT_EQ(std::size(array).get(), 3);
		EXPECT_EQ(std::size(other).get(), 0);
	}
	EXPECT_EQ(no_default_constructible_type::expected_dtor, 0);
}