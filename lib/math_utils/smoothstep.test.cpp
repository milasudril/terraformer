//@	{"target":{"name":"smoothstep.test"}}

#include "./smoothstep.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_smoothstep)
{
	EXPECT_EQ(terraformer::smoothstep(-2.0f), 0.0f);
	EXPECT_EQ(terraformer::smoothstep(-1.0f), 0.0f);
	EXPECT_EQ(terraformer::smoothstep(0.0f), 0.5f);
	EXPECT_EQ(terraformer::smoothstep(1.0f), 1.0f);
	EXPECT_EQ(terraformer::smoothstep(2.0f), 1.0f);
}