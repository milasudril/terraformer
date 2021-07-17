//@	{"target":{"name":"image.test"}}

#include "./image.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(CreateFromDataBlock)
{
	::Image img{3u, 2u};
	EXPECT_EQ(img.width(), 3);
	EXPECT_EQ(img.height(), 2);
	EXPECT_EQ(area(img), 6);

	auto ptr   = img.pixels().begin();
	*(ptr + 0) = ::red();
	*(ptr + 1) = ::green();
	*(ptr + 2) = ::blue();
	*(ptr + 3) = ::cyan();
	*(ptr + 4) = ::magenta();
	*(ptr + 5) = ::yellow();

	EXPECT_EQ(img(0, 0).red(), 1.0f);
	EXPECT_EQ(img(0, 0).green(), 0.0f);
	EXPECT_EQ(img(0, 0).blue(), 0.0f);

	EXPECT_EQ(img(1, 0).red(), 0.0f);
	EXPECT_EQ(img(1, 0).green(), 1.0f);
	EXPECT_EQ(img(1, 0).blue(), 0.0f);

	EXPECT_EQ(img(2, 0).red(), 0.0f);
	EXPECT_EQ(img(2, 0).green(), 0.0f);
	EXPECT_EQ(img(2, 0).blue(), 1.0f);

	EXPECT_EQ(img(0, 1).red(), 0.0f);
	EXPECT_EQ(img(0, 1).green(), 1.0f);
	EXPECT_EQ(img(0, 1).blue(), 1.0f);

	EXPECT_EQ(img(1, 1).red(), 1.0f);
	EXPECT_EQ(img(1, 1).green(), 0.0f);
	EXPECT_EQ(img(1, 1).blue(), 1.0f);

	EXPECT_EQ(img(2, 1).red(), 1.0f);
	EXPECT_EQ(img(2, 1).green(), 1.0f);
	EXPECT_EQ(img(2, 1).blue(), 0.0f);
}

TESTCASE(CreateEmpty)
{
	::Image img{3u, 2u};
	EXPECT_EQ(img.width(), 3);
	EXPECT_EQ(img.height(), 2);
}