//@	{"target":{"name":"chunk_by_chunk_count_view.test"}}

#include "./chunk_by_chunk_count_view.hpp"
#include "testfwk/validation.hpp"

#include <ranges>
#include <testfwk/testfwk.hpp>

TESTCASE(chunk_by_chunk_count_view_with_no_reminder)
{

	terraformer::chunk_by_chunk_count_view chunks{std::ranges::iota_view{1, 16}, 5};
	{
		int k = 1;
		for(auto x : chunks.base())
		{
			REQUIRE_EQ(x, k);
			++k;
		}
		REQUIRE_EQ(k, 16);
	}

	EXPECT_EQ(chunks.chunk_size(), 3);
	EXPECT_EQ(chunks.reminder(), 0);
	EXPECT_EQ(std::size(chunks), 5);

	size_t k = 0;
	size_t total_size = 0;
	auto const base = chunks.base();
	auto base_ptr = std::begin(base);
	for(auto x : chunks)
	{
		EXPECT_EQ(std::size(x), 3);
		for(auto y: x)
		{
			EXPECT_EQ(y, *base_ptr);
			++base_ptr;
		}

		total_size += std::size(x);
		++k;
	}
	EXPECT_EQ(k, std::size(chunks));
	EXPECT_EQ(total_size, std::size(chunks.base()));
}

TESTCASE(chunk_by_chunk_count_view_with_max_reminder)
{
	terraformer::chunk_by_chunk_count_view chunks{std::ranges::iota_view{1, 20}, 5};
	EXPECT_EQ(chunks.chunk_size(), 3);
	EXPECT_EQ(chunks.reminder(), 4);
	EXPECT_EQ(std::size(chunks), 5);
	EXPECT_EQ(std::size(chunks.base()), 19);

	size_t k = 0;
	size_t total_size = 0;
	auto const base = chunks.base();
	auto base_ptr = std::begin(base);
	for(auto x : chunks)
	{
		if(k < 4)
		{ EXPECT_EQ(std::size(x), 4); }
		else
		{ EXPECT_EQ(std::size(x), 3); }

		for(auto y: x)
		{
			EXPECT_EQ(y, *base_ptr);
			++base_ptr;
		}

		total_size += std::size(x);
		++k;
	}
	EXPECT_EQ(k, std::size(chunks));
	EXPECT_EQ(total_size, std::size(chunks.base()));
}
