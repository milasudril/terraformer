//@	{"target":{"name":"mathutils.test"}}

#include "./mathutils.hpp"

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

TESTCASE(Unity)
{
	constexpr auto val = unity<int>();
	static_assert(std::is_same_v<decltype(val), int const>);
	EXPECT_EQ(val, 1);
}

TESTCASE(Zero)
{
	constexpr auto val = zero<int>();
	static_assert(std::is_same_v<decltype(val), int const>);
	EXPECT_EQ(val, 0);
}

TESTCASE(SizeDefaultHeight)
{
	::Size box{2, 4};
	EXPECT_EQ(box.width(), 2);
	EXPECT_EQ(box.depth(), 4);
	EXPECT_EQ(box.height(), 1);
	EXPECT_EQ(volume(box), 8);
}

TESTCASE(Size)
{
	::Size box{2, 3, 4};
	EXPECT_EQ(box.width(), 2);
	EXPECT_EQ(box.depth(), 3);
	EXPECT_EQ(box.height(), 4);
	EXPECT_EQ(volume(box), 24);
}