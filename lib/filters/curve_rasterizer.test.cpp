//@{"target":{"name":"curve_rasterizer.test"}}

#include "./curve_rasterizer.hpp"
#include "lib/pixel_store/image.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_draw_curve_thickness_1)
{
	terraformer::grayscale_image img{8, 8};

	terraformer::location loc{4.0f, 3.0f, 1.0f};

	draw(img.pixels(), loc[0], loc[1], loc[2], 1.0f, [](float x, float y){
		return x*x + y*y <= 1.0f? 1.0f : 0.0f;});

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			if(k == 3 && l == 4)
			{ EXPECT_EQ(img(l, img.height() - 1 - k), 1.0f); }
			else
			{ EXPECT_EQ(img(l, img.height() - 1 - k), 0.0f); }
		}
	}
}

TESTCASE(terraformer_draw_curve_thickness_2)
{
	terraformer::grayscale_image img{8, 8};

	terraformer::location loc{4.0f, 3.0f, 1.0f};

	draw(img.pixels(), loc[0], loc[1], loc[2], 2.0f, [](float x, float y){
		return x*x + y*y <= 1.0f? 1.0f : 0.0f;});

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			if((k >= 2 && k <= 3) && (l >= 3 && l <= 4))
			{ EXPECT_EQ(img(l, img.height() - 1 - k), 1.0f); }
			else
			{ EXPECT_EQ(img(l, img.height() - 1 - k), 0.0f); }
		}
	}
}

TESTCASE(terraformer_draw_curve_thickness_2_at_half_pixel)
{
	terraformer::grayscale_image img{8, 8};

	terraformer::location loc{4.0f, 3.0f, 1.0f};

	loc += terraformer::displacement{0.5f, 0.5f, 0.0f};

	draw(img.pixels(), loc[0], loc[1], loc[2], 2.0f, [](float x, float y){
		return x*x + y*y <= 1.0f? 1.0f : 0.0f;});

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			if((k >= 3 && k <= 4) && (l >= 4 && l <= 5))
			{ EXPECT_EQ(img(l, img.height() - 1 - k), 1.0f); }
			else
			{ EXPECT_EQ(img(l, img.height() - 1 - k), 0.0f); }
		}
	}
}

TESTCASE(terraformer_draw_curve_thickness_3)
{
	terraformer::grayscale_image img{8, 8};

	terraformer::location loc{4.0f, 3.0f, 1.0f};

	draw(img.pixels(), loc[0], loc[1], loc[2], 3.0f, [](float x, float y){
		return x*x + y*y <= 1.0f? 1.0f : 0.0f;});

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			if((k >= 2 && k <= 4) && (l >= 3 && l <= 5))
			{ EXPECT_EQ(img(l, img.height() - 1 - k), 1.0f); }
			else
			{ EXPECT_EQ(img(l, img.height() - 1 - k), 0.0f); }
		}
	}
}

TESTCASE(terraformer_draw_curve_thickness_4)
{
	terraformer::grayscale_image img{8, 8};

	terraformer::location loc{4.0f, 3.0f, 1.0f};

	draw(img.pixels(), loc[0], loc[1], loc[2], 4.0f, [](float x, float y){
		return x*x + y*y <= 1.0f? 1.0f : 0.0f;});

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			if(k == 1 || k == 4)
			{
				if(l >= 3 && l <= 4)
				{ EXPECT_EQ(img(l, img.height() - 1 - k), 1.0f); }
				else
				{ EXPECT_EQ(img(l, img.height() - 1 - k), 0.0f); }
			}
			else
			if(k >= 2 && k <= 3)
			{
				if(l >= 2 && l <= 5)
				{ EXPECT_EQ(img(l, img.height() - 1 - k), 1.0f); }
				else
				{ EXPECT_EQ(img(l, img.height() - 1 - k), 0.0f); }
			}
			else
			{
				EXPECT_EQ(img(l, img.height() - 1 - k), 0.0f);
			}
		}
	}
}

TESTCASE(terraformer_draw_curve_thickness_5)
{
	terraformer::grayscale_image img{8, 8};

	terraformer::location loc{4.0f, 3.0f, 1.0f};

	draw(img.pixels(), loc[0], loc[1], loc[2], 5.0f, [](float x, float y){
		return x*x + y*y <= 1.0f? 1.0f : 0.0f;});

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			if(k == 1 || k == 5)
			{
				if(l >= 3 && l <= 5)
				{ EXPECT_EQ(img(l, img.height() - 1 - k), 1.0f); }
				else
				{ EXPECT_EQ(img(l, img.height() - 1 - k), 0.0f); }
			}
			else
			if(k >= 2 && k <= 4)
			{
				if(l >= 2 && l <= 6)
				{ EXPECT_EQ(img(l, img.height() - 1 - k), 1.0f); }
				else
				{ EXPECT_EQ(img(l, img.height() - 1 - k), 0.0f); }
			}
			else
			{
				EXPECT_EQ(img(l, img.height() - 1 - k), 0.0f);
			}
		}
	}
}

TESTCASE(terraformer_draw_point_thickness_4_at_corner)
{
	terraformer::grayscale_image img{8, 8};

	terraformer::location loc{0.0f, 0.0f, 1.0f};

	draw(img.pixels(), loc[0], loc[1], loc[2], 4.0f, [](float x, float y){
		return x*x + y*y <= 1.0f? 1.0f : 0.0f;});

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			if(k <= 1)
			{
				if(l <=2 || l == 7)
				{ EXPECT_EQ(img(l, img.height() - 1 - k), 1.0f); }
				else
				{ EXPECT_EQ(img(l, img.height() - 1 - k), 0.0f); }
			}
			else
			if(k == 2 || k == 7)
			{
				if(l <= 1)
				{ EXPECT_EQ(img(l, img.height() - 1 - k), 1.0f); }
				else
				{ EXPECT_EQ(img(l, img.height() - 1 - k), 0.0f); }
			}
		}
	}
}

TESTCASE(terraformer_draw_curve_dx_largest_greater_than_zero)
{
	terraformer::grayscale_image img{8, 8};

	std::array<terraformer::location, 2> loc{
		terraformer::location{1.0f, 1.0f, 1.0f},
		terraformer::location{7.0f, 4.0f, 4.0f}};

	draw_as_line_segments(img.pixels(), loc, 1.0f);

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			printf("%.8g ",img(l, img.height() - 1 - k));
		}
		putchar('\n');
	}
}

TESTCASE(terraformer_draw_curve_dx_largest_less_than_zero)
{
	terraformer::grayscale_image img{8, 8};

	std::array<terraformer::location, 2> loc{
		terraformer::location{7.0f, 1.0f, 1.0f},
		terraformer::location{1.0f, 4.0f, -2.0f}};

	draw_as_line_segments(img.pixels(), loc, 1.0f);

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			printf("%.8g ",img(l, img.height() - 1 - k));
		}
		putchar('\n');
	}
}

TESTCASE(terraformer_draw_curve_dy_largest_greater_than_zero)
{
	terraformer::grayscale_image img{8, 8};

	std::array<terraformer::location, 2> loc{
		terraformer::location{1.0f, 1.0f, 1.0f},
		terraformer::location{4.0f, 7.0f, -2.0f}};

	draw_as_line_segments(img.pixels(), loc, 1.0f);

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			printf("%.8g ",img(l, img.height() - 1 - k));
		}
		putchar('\n');
	}
}

TESTCASE(terraformer_draw_curve_dy_largest_less_than_zero)
{
	terraformer::grayscale_image img{8, 8};

	std::array<terraformer::location, 2> loc{
		terraformer::location{1.0f, 7.0f, 1.0f},
		terraformer::location{4.0f, 1.0f, 4.0f}};

	draw_as_line_segments(img.pixels(), loc, 1.0f);

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			printf("%.8g ",img(l, img.height() - 1 - k));
		}
		putchar('\n');
	}
}