//@	{"target":{"name":"vec_t.test"}}

#include "./vec_t.hpp"
#include "testfwk/testfwk.hpp"

TESTCASE(VectorCast)
{
	vec4_t<float> const v{1.0f, 1.75f, 1.25f, -1.5f};
	auto const v1 = vector_cast<int>(v);
	static_assert(std::is_same_v<decltype(v1), vec4_t<int> const>);
	vec4_t<int> const expected{1, 1, 1, -1};
	for(int k = 0; k < 4; ++k)
	{ EXPECT_EQ(v1[k], expected[k]); }
}