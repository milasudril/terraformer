//@	{"target":{"name":"text_shaper.test"}}

#include "./text_shaper.hpp"

#include "./font_mapper.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_font_handling_shape_text)
{
	terraformer::ui::font_handling::font_mapper fonts;
	auto const fontfile = fonts.get_path("sans-serif");
	terraformer::ui::font_handling::glyph_renderer renderer{fontfile.c_str()};
	terraformer::ui::font_handling::shaper_font the_font{24, renderer};
	terraformer::ui::font_handling::text_shaper shaper{};

	auto result = shaper.append(u8"Yxmördaren Julia Blomqvist på fäktning i Schweiz")
		.with(hb_script_t::HB_SCRIPT_LATIN)
		.with(hb_direction_t::HB_DIRECTION_LTR)
		.with(hb_language_from_string("sv-SE", -1))
		.run(the_font);

	EXPECT_EQ(result.glyph_count, 48);
	EXPECT_EQ(&result.renderer.get(), &renderer);

	for(size_t k = 0; k != result.glyph_count; ++k)
	{
		auto const& gi = result.glyph_info[k];
//		auto const& gp = result.glyph_pos[k];
		printf("%d ", gi.codepoint);
	}
	putchar('\n');

}