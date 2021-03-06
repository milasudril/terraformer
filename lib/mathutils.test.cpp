//@	{"target":{"name":"mathutils.test"}}

#include "./mathutils.hpp"

#include "testfwk/testfwk.hpp"

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

TESTCASE(PolygonChain)
{
	::PolygonChain polychain{::Point{1.0f, 2.0f, 3.0f}, ::Point{4.0f, 5.0f, 6.0f}};
	translate(polychain, ::Vector{1.0f, 1.0f, 1.0f});

	EXPECT_EQ(std::begin(polychain.vertices())->x(), 2.0f);
	EXPECT_EQ(std::begin(polychain.vertices())->y(), 3.0f);
	EXPECT_EQ(std::begin(polychain.vertices())->z(), 4.0f);

	EXPECT_EQ((std::end(polychain.vertices()) - 1)->x(), 5.0f);
	EXPECT_EQ((std::end(polychain.vertices()) - 1)->y(), 6.0f);
	EXPECT_EQ((std::end(polychain.vertices()) - 1)->z(), 7.0f);
}