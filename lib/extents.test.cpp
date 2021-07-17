//@	{"target":{"name":"extents.test"}}

#include "./extents.hpp"

#include "testfwk/testfwk.hpp"

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
