//@	{"target":{"name":"interleave.test"}}

#include "./interleave.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_interleave_odd)
{
	std::array<int, 9> vals{1, 2, 3, 4, 5, 6, 7, 8, 9};
	auto res = terraformer::interleave(std::span<int const>{vals});
	EXPECT_EQ(std::size(res), std::size(vals));

	EXPECT_EQ(res[0], 1);
	EXPECT_EQ(res[1], 3);
	EXPECT_EQ(res[2], 5);
	EXPECT_EQ(res[3], 7);
	EXPECT_EQ(res[4], 9);
	EXPECT_EQ(res[5], 2);
	EXPECT_EQ(res[6], 4);
	EXPECT_EQ(res[7], 6);
	EXPECT_EQ(res[8], 8);
}

TESTCASE(terraformer_interleave_even)
{
	std::array<int, 8> vals{1, 2, 3, 4, 5, 6, 7, 8};
	auto res = terraformer::interleave(std::span<int const>{vals});
	EXPECT_EQ(std::size(res), std::size(vals));

	EXPECT_EQ(res[0], 1);
	EXPECT_EQ(res[1], 3);
	EXPECT_EQ(res[2], 5);
	EXPECT_EQ(res[3], 7);
	EXPECT_EQ(res[4], 2);
	EXPECT_EQ(res[5], 4);
	EXPECT_EQ(res[6], 6);
	EXPECT_EQ(res[7], 8);
}