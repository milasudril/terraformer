//@	{"target":{"name":"image.test"}}

#include "./image.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(CreateFromDataBlock)
{
	terraformer::image img{3u, 2u};
	EXPECT_EQ(img.width(), 3);
	EXPECT_EQ(img.height(), 2);

	auto ptr   = img.pixels().begin();
	*(ptr + 0) = terraformer::red();
	*(ptr + 1) = terraformer::green();
	*(ptr + 2) = terraformer::blue();
	*(ptr + 3) = terraformer::cyan();
	*(ptr + 4) = terraformer::magenta();
	*(ptr + 5) = terraformer::yellow();

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
	terraformer::image img{3u, 2u};
	EXPECT_EQ(img.width(), 3);
	EXPECT_EQ(img.height(), 2);
}