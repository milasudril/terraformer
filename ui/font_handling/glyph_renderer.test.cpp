//@	{"target":{"name":"glyph_renderer.test"}}

#include "./glyph_renderer.hpp"

#include "./font_mapper.hpp"
#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_font_handling_glyph_renderer_load_latin1)
{
	terraformer::ui::font_handling::font_mapper fonts;
	auto const fontfile = fonts.get_path("DejaVu Sans");

	terraformer::ui::font_handling::glyph_renderer renderer{fontfile.c_str()};

	// Load in cold state
	auto& glyph_1 = renderer.get_glyph(24, terraformer::ui::font_handling::codepoint{65});

	// Load with same size as before
	auto& glyph_2 = renderer.get_glyph(24, terraformer::ui::font_handling::codepoint{66});
	EXPECT_NE(&glyph_1, &glyph_2);

	// Load with different size as before
	auto& glyph_3 = renderer.get_glyph(16, terraformer::ui::font_handling::codepoint{66});
	EXPECT_NE(&glyph_1, &glyph_3);
	EXPECT_NE(&glyph_2, &glyph_3);

	// Fetch all loaded glyphs
	auto& glyph_4 = renderer.get_glyph(24, terraformer::ui::font_handling::codepoint{65});
	auto& glyph_5 = renderer.get_glyph(24, terraformer::ui::font_handling::codepoint{66});
	auto& glyph_6 = renderer.get_glyph(16, terraformer::ui::font_handling::codepoint{66});

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

TESTCASE(terraformer_ui_font_handling_glyph_renderer_load_outside_latin_1)
{
	terraformer::ui::font_handling::font_mapper fonts;
	auto const fontfile = fonts.get_path("DejaVu Sans");

	terraformer::ui::font_handling::glyph_renderer renderer{fontfile.c_str()};

	// Load in cold state
	auto& glyph_1 = renderer.get_glyph(24, terraformer::ui::font_handling::codepoint{256});

	// Load with same size as before
	auto& glyph_2 = renderer.get_glyph(24, terraformer::ui::font_handling::codepoint{257});
	EXPECT_NE(&glyph_1, &glyph_2);

	// Load with different size as before
	auto& glyph_3 = renderer.get_glyph(16, terraformer::ui::font_handling::codepoint{257});
	EXPECT_NE(&glyph_1, &glyph_3);
	EXPECT_NE(&glyph_2, &glyph_3);

	// Fetch all loaded glyphs
	auto& glyph_4 = renderer.get_glyph(24, terraformer::ui::font_handling::codepoint{256});
	auto& glyph_5 = renderer.get_glyph(24, terraformer::ui::font_handling::codepoint{257});
	auto& glyph_6 = renderer.get_glyph(16, terraformer::ui::font_handling::codepoint{257});

#if 0
	// TODO: Must compare by value since unordered_map may move elements

	EXPECT_EQ(&glyph_1, &glyph_4);
	EXPECT_NE(&glyph_1, &glyph_5);
	EXPECT_NE(&glyph_1, &glyph_6);

	EXPECT_EQ(&glyph_2, &glyph_5);
	EXPECT_NE(&glyph_2, &glyph_6);
	EXPECT_NE(&glyph_2, &glyph_4);

	EXPECT_EQ(&glyph_3, &glyph_6);
	EXPECT_NE(&glyph_3, &glyph_4);
	EXPECT_NE(&glyph_3, &glyph_5);
#endif

	EXPECT_NE(&glyph_4, &glyph_5);
	EXPECT_NE(&glyph_5, &glyph_6);
	EXPECT_NE(&glyph_6, &glyph_4);
}