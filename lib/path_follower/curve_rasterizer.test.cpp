//@{"target":{"name":"curve_rasterizer.test"}}

#include "./curve_rasterizer.hpp"
#include "lib/pixel_store/image.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_draw_curve_thickness_1)
{
	terraformer::grayscale_image img{8, 8};

	terraformer::location loc{4.0f, 3.0f, 1.0f};

	draw(loc, img.pixels(), [](float x, float y, float z){
		return x*x + y*y <= 1.0f? z : 0.0f;
	}, [](auto){ return 1.0f; });

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			if(k == 3 && l == 4)
			{ EXPECT_EQ(img(l, k), 1.0f); }
			else
			{ EXPECT_EQ(img(l, k), 0.0f); }
		}
	}
}

TESTCASE(terraformer_draw_curve_thickness_2)
{
	terraformer::grayscale_image img{8, 8};

	terraformer::location loc{4.0f, 3.0f, 1.0f};

	draw(loc, img.pixels(), [](float x, float y, float z){
		return x*x + y*y <= 1.0f? z : 0.0f;
	}, [](auto){ return 2.0f; });

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			if((k >= 2 && k <= 3) && (l >= 3 && l <= 4))
			{ EXPECT_EQ(img(l, k), 1.0f); }
			else
			{ EXPECT_EQ(img(l, k), 0.0f); }
		}
	}
}

TESTCASE(terraformer_draw_curve_thickness_2_at_half_pixel)
{
	terraformer::grayscale_image img{8, 8};

	terraformer::location loc{4.0f, 3.0f, 1.0f};

	loc += terraformer::displacement{0.5f, 0.5f, 0.0f};

	draw(loc, img.pixels(), [](float x, float y, float z){
		return x*x + y*y <= 1.0f? z : 0.0f;
	}, [](auto){ return 2.0f; });

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			if((k >= 3 && k <= 4) && (l >= 4 && l <= 5))
			{ EXPECT_EQ(img(l, k), 1.0f); }
			else
			{ EXPECT_EQ(img(l, k), 0.0f); }
		}
	}
}

TESTCASE(terraformer_draw_curve_thickness_3)
{
	terraformer::grayscale_image img{8, 8};

	terraformer::location loc{4.0f, 3.0f, 1.0f};

	draw(loc, img.pixels(), [](float x, float y, float z){
		return x*x + y*y <= 1.0f? z : 0.0f;
	}, [](auto){ return 3.0f; });

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			if((k >= 2 && k <= 4) && (l >= 3 && l <= 5))
			{ EXPECT_EQ(img(l, k), 1.0f); }
			else
			{ EXPECT_EQ(img(l, k), 0.0f); }
		}
	}
}

TESTCASE(terraformer_draw_curve_thickness_4)
{
	terraformer::grayscale_image img{8, 8};

	terraformer::location loc{4.0f, 3.0f, 1.0f};

	draw(loc, img.pixels(), [](float x, float y, float z){
		return x*x + y*y <= 1.0f? z : 0.0f;
	}, [](auto){ return 4.0f; });

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			if(k == 1 || k == 4)
			{
				if(l >= 3 && l <= 4)
				{ EXPECT_EQ(img(l, k), 1.0f); }
				else
				{ EXPECT_EQ(img(l, k), 0.0f); }
			}
			else
			if(k >= 2 && k <= 3)
			{
				if(l >= 2 && l <= 5)
				{ EXPECT_EQ(img(l, k), 1.0f); }
				else
				{ EXPECT_EQ(img(l, k), 0.0f); }
			}
			else
			{
				EXPECT_EQ(img(l, k), 0.0f);
			}
		}
	}
}

TESTCASE(terraformer_draw_curve_thickness_5)
{
	terraformer::grayscale_image img{8, 8};

	terraformer::location loc{4.0f, 3.0f, 1.0f};

	draw(loc, img.pixels(), [](float x, float y, float z){
		return x*x + y*y <= 1.0f? z : 0.0f;
	}, [](auto){ return 5.0f; });

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			if(k == 1 || k == 5)
			{
				if(l >= 3 && l <= 5)
				{ EXPECT_EQ(img(l, k), 1.0f); }
				else
				{ EXPECT_EQ(img(l, k), 0.0f); }
			}
			else
			if(k >= 2 && k <= 4)
			{
				if(l >= 2 && l <= 6)
				{ EXPECT_EQ(img(l, k), 1.0f); }
				else
				{ EXPECT_EQ(img(l, k), 0.0f); }
			}
			else
			{
				EXPECT_EQ(img(l, k), 0.0f);
			}
		}
	}
}