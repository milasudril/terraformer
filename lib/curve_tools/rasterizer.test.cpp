//@{"target":{"name":"rasterizer.test"}}

#include "./rasterizer.hpp"
#include "lib/pixel_store/image.hpp"

#include "testfwk/testfwk.hpp"

namespace
{
	struct my_brush
	{
		float radius;

		void begin_pixel(float, float, float, terraformer::array_index<terraformer::location>){}

		float get_radius() const
		{ return radius; }

		float get_pixel_value(float, float new_val, float xi, float eta) const
		{
			auto const r = std::sqrt(xi*xi + eta*eta);
			return r < 1.0f ? new_val : 0.0f;
		}
	};
}

TESTCASE(terraformer_draw_curve_thickness_1)
{
	terraformer::grayscale_image img{8, 8};

	terraformer::location loc{4.0f, 3.0f, 1.0f};

	paint(img.pixels(),
		 terraformer::paint_params{
			.x = loc[0],
			.y = loc[1],
			.value = loc[2],
			.brush = my_brush{
				.radius = 0.5f
			}
		}
	);

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

	paint(img.pixels(),
		terraformer::paint_params{
			.x = loc[0],
			.y = loc[1],
			.value = loc[2],
			.brush = my_brush{
				.radius = 1.0f
			}
		}
	);

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

	paint(img.pixels(),
		terraformer::paint_params{
			.x = loc[0],
			.y = loc[1],
			.value = loc[2],
			.brush = my_brush{
				.radius = 1.0f
			}
		}
	);

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

	paint(img.pixels(),
		terraformer::paint_params{
			.x = loc[0],
			.y = loc[1],
			.value = loc[2],
			.brush = my_brush{
				.radius = 1.5f
			}
		}
	);

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

	paint(img.pixels(),
		terraformer::paint_params{
			.x = loc[0],
			.y = loc[1],
			.value = loc[2],
			.brush = my_brush{
 				.radius = 2.0f
			}
		}
	);

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

	paint(img.pixels(),
		terraformer::paint_params{
			.x = loc[0],
			.y = loc[1],
			.value = loc[2],
			.brush = my_brush{
 				.radius = 2.5f
			}
		}
	);

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

namespace
{
	struct pixel_counter
	{
		void begin_pixel(float, float, float, terraformer::array_index<terraformer::location>){}

		float get_radius() const
		{ return 0.5f; }

		template<class Ignore>
		float get_pixel_value(float old_val, Ignore&&...) const
		{
			return old_val + 1.0f;
		}
	};
}

TESTCASE(terraformer_draw_curve_thickness_1_pixels_visited_once)
{
	terraformer::grayscale_image img{8, 8};

	constexpr std::array<terraformer::location, 3> loc{
		terraformer::location{0.0f, 0.0f, 1.0f},
		terraformer::location{7.0f, 0.0f, 1.0f},
		terraformer::location{0.0f, 3.5f, 1.0f}
	};

	draw(
		img.pixels(),
		terraformer::span{std::begin(loc), std::end(loc)},
		terraformer::line_segment_draw_params{
			.value = 1.0f,
			.scale = 1.0f,
			.brush = pixel_counter{}
		}
	);

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			EXPECT_LE(img(l, k), 1.0f)
		}
	}
}

TESTCASE(terraformer_draw_curve_thickness_1_pixels_visited_once_scaled)
{
	terraformer::grayscale_image img{8, 8};

	constexpr std::array<terraformer::location, 3> loc{
		terraformer::location{0.0f, 0.0f, 1.0f},
		terraformer::location{15.0f, 0.0f, 1.0f},
		terraformer::location{0.0f, 9.0f, 1.0f}
	};

	draw(
		img.pixels(),
		terraformer::span{std::begin(loc), std::end(loc)},
		terraformer::line_segment_draw_params{
			.value = 1.0f,
			.scale = 2.0f,
			.brush = pixel_counter{}
		}
	);

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{ EXPECT_LE(img(l, k), 1.0f); }
	}
}

#if 0
//TODO
TESTCASE(terraformer_draw_point_thickness_4_at_corner)
{
	terraformer::grayscale_image img{8, 8};

	terraformer::location loc{0.0f, 0.0f, 1.0f};

	paint(img.pixels(),
		terraformer::paint_params{
			.x = loc[0],
			.y = loc[1],
			.value = loc[2],
			.brush_diameter = 4.0f
		});

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			if(k <= 1)
			{
				if(l <=2 || l == 7)
				{ EXPECT_EQ(img(l, k), 1.0f); }
				else
				{ EXPECT_EQ(img(l, k), 0.0f); }
			}
			else
			if(k == 2 || k == 7)
			{
				if(l <= 1)
				{ EXPECT_EQ(img(l, k), 1.0f); }
				else
				{ EXPECT_EQ(img(l, k), 0.0f); }
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

	draw(img.pixels(), loc, terraformer::line_segment_draw_params{.value = 1.0f});

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			printf("%.8g ",img(l, k));
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

	draw(img.pixels(), loc, terraformer::line_segment_draw_params{.value = 1.0f});

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			printf("%.8g ",img(l, k));
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

	draw(img.pixels(), loc, terraformer::line_segment_draw_params{.value = 1.0f});

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			printf("%.8g ",img(l, k));
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

	draw(img.pixels(), loc, terraformer::line_segment_draw_params{.value = 1.0f});

	for(uint32_t k = 0; k != img.height(); ++k)
	{
		for(uint32_t l = 0; l != img.width(); ++l)
		{
			printf("%.8g ",img(l, k));
		}
		putchar('\n');
	}
}
#endif