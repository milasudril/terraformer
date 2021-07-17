//@	{"target":{"name":"mathutils.test"}}

#include "./mathutils.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(VectorCast)
{
	vec4_t<float> const v{1.0f, 1.75f, 1.25f, -1.5f};
	auto const v1 = vector_cast<int>(v);
	vec4_t<int> const expected{1, 1, 1, -1};
	for(int k = 0; k < 4; ++k)
	{ EXPECT_EQ(v1[k], expected[k]); }
}

