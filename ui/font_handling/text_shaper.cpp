//@	{"target":{"name":"text_shaper.o"}}

#include "./text_shaper.hpp"

terraformer::basic_image<uint8_t>
terraformer::ui::font_handling::render(shaping_result const& result)
{
	uint64_t width = 0;
	uint64_t height = 0;
	auto const n = result.glyph_count;
	auto const glyph_info = result.glyph_info;
	auto const glyph_pos = result.glyph_pos;
	for(size_t i = 0; i != n; ++i)
	{
		auto const& glyph = get_glyph(result, glyph_index{glyph_info[i].codepoint});
		width += (i != n - 1)? glyph_pos[i].x_advance : glyph.image.width()*64;
		height += (i != n - 1)? glyph_pos[i].y_advance : glyph.image.height()*64;
	}

	auto const& renderer = result.renderer.get();

	height = std::max(static_cast<uint64_t>(renderer.get_global_glyph_height()), height);
	width = std::max(static_cast<uint64_t>(renderer.get_global_glyph_width()), width);

	terraformer::basic_image<uint8_t> ret{
		narrowing_cast<uint32_t>(width/64),
		narrowing_cast<uint32_t>(height/64)
	};

	return ret;
}