//@	{"target":{"name":"text_shaper.test"}}

#include "./text_shaper.hpp"

#include "./font_mapper.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_font_handling_shape_text)
{
	terraformer::ui::font_handling::font_mapper fonts;
	auto const fontfile = fonts.get_path("serif");
	terraformer::ui::font_handling::glyph_renderer renderer{fontfile.c_str()};
	terraformer::ui::font_handling::font the_font{24, renderer};
	terraformer::ui::font_handling::text_shaper shaper{};

	auto result = shaper.append(u8"Yxmördaren Julia Blomqvist på fäktning i Schweiz ff")
		.with(hb_script_t::HB_SCRIPT_LATIN)
		.with(hb_direction_t::HB_DIRECTION_TTB)
		.with(hb_language_from_string("sv-SE", -1))
		.run(the_font);

	EXPECT_EQ(result.glyph_count, 50);
	EXPECT_EQ(&result.renderer.get(), &renderer);

	auto img = render(result);
	FILE* dump = fopen("/dev/shm/slask.data", "wb");
	printf("%u %u\n", img.width(), img.height());
	fwrite(std::data(img.pixels()), 1, img.width()*img.height(), dump);
	fclose(dump);
}