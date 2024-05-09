//@	{"target":{"name":"glyph_renderer.test"}}

#include "./glyph_renderer.hpp"

#include "./font_mapper.hpp"
#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_font_handling_glyph_renderer_init)
{
	terraformer::ui::font_handling::font_mapper fonts;
	auto const fontfile = fonts.get_path("DejaVu Sans");

	terraformer::ui::font_handling::glyph_renderer renderer{fontfile.c_str()};

	auto& glyph = renderer.set_font_size(16).get_glyph(terraformer::ui::font_handling::codepoint{65});

	printf("%p\n", &glyph);
}