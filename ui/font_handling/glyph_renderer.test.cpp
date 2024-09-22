//@	{"target":{"name":"glyph_renderer.test"}}

#include "./glyph_renderer.hpp"

#include "./font_mapper.hpp"
#include <testfwk/testfwk.hpp>

namespace
{
	void dump_glyph(terraformer::ui::font_handling::glyph const& g)
	{
		for(uint32_t y = 0; y != g.image.height(); ++y)
		{
			for(uint32_t x = 0; x != g.image.width(); ++x)
			{
				if(g.image(x, y) > 128)
				{ putchar('*'); }
				else
				{ putchar(' '); }
			}
			putchar('\n');
		}
		putchar('\n');
	}
}

TESTCASE(terraformer_ui_font_handling_glyph_renderer_load_small_index)
{
	terraformer::ui::font_handling::font_mapper fonts;
	auto const fontfile = fonts.get_path("sans-serif");

	terraformer::ui::font_handling::glyph_renderer renderer{fontfile};

	// Load in cold state
	auto& glyph_1 = renderer.set_font_size(24)
		.get_glyph(terraformer::ui::font_handling::codepoint{65});
	dump_glyph(glyph_1);
	printf("%lu\n", renderer.get_global_glyph_height());

	// Load with same size as before
	auto& glyph_2 = renderer.get_glyph(terraformer::ui::font_handling::codepoint{66});
	EXPECT_NE(&glyph_1, &glyph_2);
	dump_glyph(glyph_2);

	// Load with different size as before
	auto& glyph_3 = renderer.set_font_size(16)
		.get_glyph(terraformer::ui::font_handling::codepoint{66});
	EXPECT_NE(&glyph_1, &glyph_3);
	EXPECT_NE(&glyph_2, &glyph_3);
	dump_glyph(glyph_3);
	printf("%lu\n", renderer.get_global_glyph_height());

	// Fetch all loaded glyphs
	auto& glyph_4 = renderer.set_font_size(24).get_glyph(terraformer::ui::font_handling::codepoint{65});
	auto& glyph_5 = renderer.get_glyph(terraformer::ui::font_handling::codepoint{66});
	auto& glyph_6 = renderer.set_font_size(16).get_glyph(terraformer::ui::font_handling::codepoint{66});

	EXPECT_EQ(&glyph_1, &glyph_4);
	EXPECT_NE(&glyph_1, &glyph_5);
	EXPECT_NE(&glyph_1, &glyph_6);

	EXPECT_EQ(&glyph_2, &glyph_5);
	EXPECT_NE(&glyph_2, &glyph_6);
	EXPECT_NE(&glyph_2, &glyph_4);

	EXPECT_EQ(&glyph_3, &glyph_6);
	EXPECT_NE(&glyph_3, &glyph_4);
	EXPECT_NE(&glyph_3, &glyph_5);

	EXPECT_NE(&glyph_4, &glyph_5);
	EXPECT_NE(&glyph_5, &glyph_6);
	EXPECT_NE(&glyph_6, &glyph_4);
}