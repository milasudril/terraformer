//@	{"target":{"name":"length.test"}}

#include "./length.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_curve_length)
{
	std::array<terraformer::location, 4> locs{
		terraformer::location{0.0f, 0.0f, 0.0f},
		terraformer::location{4.0f, 0.0f, 0.0f},
		terraformer::location{4.0f, 3.0f, 0.0f},
		terraformer::location{0.0f, 0.0f, 0.0f}
	};
	auto res = terraformer::curve_length(locs);
	EXPECT_EQ(res, 3.0f + 4.0f + 5.0f);
}