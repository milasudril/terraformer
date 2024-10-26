//@	{"target":{"name":"span.test"}}

#include "./span.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_span_test_all_members)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};

	terraformer::span span{std::begin(data), std::end(data)};

	EXPECT_EQ(std::size(span).get(), std::size(data));

	{
		size_t l = 0;
		for(auto k : span.element_indices())
		{
			EXPECT_EQ(span[k], data[l]);
			++l;
		}
	}

	{
		auto l = span.element_indices().front();
		EXPECT_EQ(l.get(), 0);
		auto ptr_a = std::data(span);
		auto ptr_b = std::data(data);
		while(l != std::size(span))
		{
			EXPECT_EQ(ptr_a, ptr_b);
			++l;
			++ptr_a;
			++ptr_b;
		}
	}

	{
		auto begin = std::begin(span);
		auto end = std::end(span);
		auto other = std::begin(data);

		while(begin != end)
		{
			EXPECT_EQ(begin, other);
			++begin;
			++other;
		}
	}
}