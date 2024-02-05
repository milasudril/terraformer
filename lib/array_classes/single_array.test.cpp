//@	{"target":{"name":"single_array.test"}}

#include "./single_array.hpp"

#include <cassert>
#include <testfwk/testfwk.hpp>

namespace
{
	class my_test_type
	{
	public:
		static size_t expected_init;
		static size_t expected_move_ctor;
		static size_t expected_dtor;
		
		my_test_type(int value):m_value{value}
		{
			if(expected_init == 0)
			{	throw std::runtime_error{"Unexpected init"}; }
			
			--expected_init;
			++expected_dtor;
		}
		
		my_test_type(my_test_type&& other): m_value{other.m_value}
		{
			if(expected_move_ctor == 0)
			{ throw std::runtime_error{"Unexpected move ctor"}; }
			
			--expected_move_ctor;
			++expected_dtor;
		}
		
		~my_test_type()
		{ 
			assert(expected_dtor != 0);
			--expected_dtor;
		}

		auto value() const
		{ return m_value; }
	private:
		int m_value;
	};
	
	
	constinit size_t my_test_type::expected_init = 0;
	constinit size_t my_test_type::expected_move_ctor = 0;
	constinit size_t my_test_type::expected_dtor = 0;
}

TESTCASE(terraformer_single_array_reseve_on_empty)
{
	terraformer::single_array<my_test_type> array;
	REQUIRE_EQ(array.size(), terraformer::array_size<my_test_type>{});
	REQUIRE_EQ(array.capacity(), terraformer::array_size<my_test_type>{});
	REQUIRE_EQ(array.begin(), array.end());
	
	array.reserve(terraformer::array_size<my_test_type>(5));
	
	EXPECT_EQ(array.size(), terraformer::array_size<my_test_type>{});
	EXPECT_EQ(array.capacity(), terraformer::array_size<my_test_type>{5});
	EXPECT_EQ(array.begin(), array.end());
}

TESTCASE(terraformer_single_array_push_back)
{
	{
		terraformer::single_array<my_test_type> array;

		for(int k = 0; k != 16; ++k)
		{
			++my_test_type::expected_init;
			++my_test_type::expected_move_ctor;

			if(static_cast<size_t>(k) == array.capacity().get())
			{ my_test_type::expected_move_ctor += array.capacity().get(); }

			array.push_back(my_test_type{k});
		}
		
		EXPECT_EQ(my_test_type::expected_init, 0);
		EXPECT_EQ(my_test_type::expected_move_ctor, 0);
		
		for(auto k = array.first_element_index(); k != std::size(array); ++k)
		{ EXPECT_EQ(static_cast<size_t>(array[k].value()), k.get()); }
		
		EXPECT_EQ(std::size(array), terraformer::array_size<my_test_type>{16});
		
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
			auto span = static_cast<terraformer::span<my_test_type>>(array);
			for(auto k = array.first_element_index(); k != std::size(span); ++k)
			{ EXPECT_EQ(static_cast<size_t>(array[k].value()), k.get()); }
		}
	}

	EXPECT_EQ(my_test_type::expected_dtor, 0);
}
