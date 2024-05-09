//@	{"target":{"name":"glyph_renderer.test"}}

#include "./glyph_renderer.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_font_handling_glyph_renderer_init)
{
	EXPECT_EQ(terraformer::ui::font_handling::glyph_renderer::loader_usecount(), 0);
	{
		terraformer::ui::font_handling::glyph_renderer renderer{};
		EXPECT_EQ(renderer.loader_usecount(), 1);
		{
			terraformer::ui::font_handling::glyph_renderer renderer{};
			EXPECT_EQ(renderer.loader_usecount(), 2);
		}
		EXPECT_EQ(renderer.loader_usecount(), 1);
	}
	EXPECT_EQ(terraformer::ui::font_handling::glyph_renderer::loader_usecount(), 0);
}