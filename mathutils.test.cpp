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

TESTCASE(MakeArc)
{
	::Size box{3, 1};
	auto arc = make_arc(box);
	EXPECT_EQ(arc.radius, 5);
	::Vector arc_end{arc.radius*std::cos(arc.angle), arc.radius*std::sin(arc.angle)};
	EXPECT_EQ(arc_end.x(), arc.radius - box.depth());
	EXPECT_EQ(arc_end.y(), box.width());
	EXPECT_EQ(length(arc), arc.radius*arc.angle);
}

TESTCASE(Vector)
{
	::Vector v{3, 4, 5};
	EXPECT_EQ(v.x(), 3);
	EXPECT_EQ(v.y(), 4);
	EXPECT_EQ(v.z(), 5);
	::Vector x{1, 0, 0};
	::Vector y{0, 1, 0};
	::Vector z{0, 0, 1};

	EXPECT_EQ(dot(v, x), v.x());
	EXPECT_EQ(dot(v, y), v.y());
	EXPECT_EQ(dot(v, z), v.z());
	EXPECT_EQ(dot(v, v), 9+16+25);
	EXPECT_EQ(length_squared(v), 9+16+25);

	auto v2 = 2*v;
	EXPECT_EQ(v2.x(), 2*v.x());
	EXPECT_EQ(v2.y(), 2*v.y());
	EXPECT_EQ(v2.z(), 2*v.z());
	EXPECT_EQ(dot(v, v2), 2*dot(v,v));

	EXPECT_EQ(length(v2), 2*length(v));
	EXPECT_GT(length(v), 7.0);
	EXPECT_LT(length(v), 8.0);
	auto l = length(v);
	static_assert(std::is_same_v<decltype(l), double>);

	auto v_norm = normalized(v);
	EXPECT_LE(length(v_norm), std::nextafter(1.0, 2.0));
	EXPECT_GE(length(v_norm), std::nextafter(1.0, 0.0));

	auto v3 = v2 + v;
	EXPECT_EQ(v3.x(), v2.x() + v.x());
	EXPECT_EQ(v3.y(), v2.y() + v.y());
	EXPECT_EQ(v3.z(), v2.z() + v.z());

	auto v4 = v3 - v;
	EXPECT_EQ(v4.x(), v2.x());
	EXPECT_EQ(v4.y(), v2.y());
	EXPECT_EQ(v4.z(), v2.z());

}

TESTCASE(VectorFloat)
{
	::Vector v{3.0f, 4.0f, 5.0f};

	auto v_norm = normalized(v);
	EXPECT_LE(length(v_norm), std::nextafter(1.0f, 2.0f));
	EXPECT_GE(length(v_norm), std::nextafter(1.0f, 0.0f));
}

TESTCASE(VectorNoZ)
{
	::Vector v{3, 4};
	EXPECT_EQ(v.z(), 0);
}

TESTCASE(Point)
{
	::Point const P1{1, 2, 3};
	::Point const P2{4, 6, 8};
	EXPECT_EQ(P1.x(), 1);
	EXPECT_EQ(P1.y(), 2);
	EXPECT_EQ(P1.z(), 3);
	EXPECT_EQ(P2.x(), 4);
	EXPECT_EQ(P2.y(), 6);
	EXPECT_EQ(P2.z(), 8);

	auto const v = P2 - P1;
	static_assert(std::is_same_v<decltype(v), ::Vector<int> const>);
	EXPECT_EQ(v.x(), 3);
	EXPECT_EQ(v.y(), 4);
	EXPECT_EQ(v.z(), 5);

	EXPECT_EQ(distance(P1, P2), distance(P2, P1));
	auto const d = distance(P1, P2);
	EXPECT_GT(d, 7.0);
	EXPECT_LT(d, 8.0);

	auto const P3 = P1 + ::Vector{4, 6, 8};
	static_assert(std::is_same_v<decltype(P3), ::Point<int> const>);
	EXPECT_EQ(P3.x(), P1.x() + 4);
	EXPECT_EQ(P3.y(), P1.y() + 6);
	EXPECT_EQ(P3.z(), P1.z() + 8);

	auto const P4 = P3 - ::Vector{4, 6, 8};
	static_assert(std::is_same_v<decltype(P4), ::Point<int> const>);
	EXPECT_EQ(P4.x(), P1.x());
	EXPECT_EQ(P4.y(), P1.y());
	EXPECT_EQ(P4.z(), P1.z());
}