//@	{"target":{"name":"extents.test"}}

#include "./extents.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(ExtentsDefaultHeight)
{
	::Extents box{2, 4};
	EXPECT_EQ(box.width(), 2);
	EXPECT_EQ(box.depth(), 4);
	EXPECT_EQ(box.height(), 1);
	EXPECT_EQ(volume(box), 8);
}

TESTCASE(Extents)
{
	::Extents box{2, 3, 4};
	EXPECT_EQ(box.width(), 2);
	EXPECT_EQ(box.depth(), 3);
	EXPECT_EQ(box.height(), 4);
	EXPECT_EQ(volume(box), 24);
}
