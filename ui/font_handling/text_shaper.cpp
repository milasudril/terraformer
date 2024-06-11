//@	{"target":{"name":"text_shaper.o"}}

#include "./text_shaper.hpp"

terraformer::span_2d_extents
terraformer::ui::font_handling::compute_extents(shaping_result const& result)
{
	uint64_t width = 0;
	uint64_t height = 0;
	auto const n = result.glyph_count;
	auto const glyph_info = result.glyph_info;
	auto const glyph_pos = result.glyph_pos;
	for(size_t i = 0; i != n; ++i)
	{
		auto const& glyph = get_glyph(result, glyph_index{glyph_info[i].codepoint});
		width += (i != n - 1)?
			glyph_pos[i].x_advance :
			std::max(glyph.image.width()*64, static_cast<uint32_t>(glyph_pos[i].x_advance));
		printf("%zu ", width/64);
		height += (i != n - 1)? -glyph_pos[i].y_advance : glyph.image.height()*64;
	}

	auto const& renderer = result.renderer.get();

	height = std::max(static_cast<uint64_t>(renderer.get_global_glyph_height()), height);
	width = std::max(static_cast<uint64_t>(renderer.get_global_glyph_width()), width);

	return span_2d_extents{
		.width = narrowing_cast<uint32_t>(width/64),
		.height = narrowing_cast<uint32_t>(height/64)
	};
}

terraformer::basic_image<uint8_t>
terraformer::ui::font_handling::render(shaping_result const& result)
{
	// TODO: Fix vertical rendering

	auto const size = compute_extents(result);
	terraformer::basic_image<uint8_t> ret{size.width, size.height};

	auto const n = result.glyph_count;
	auto const glyph_info = result.glyph_info;
	auto const glyph_pos = result.glyph_pos;
	auto const ascender = result.renderer.get().get_ascender()/64;

	uint64_t cursor_x = 0;
	uint64_t cursor_y = 0;

	for(size_t i = 0; i != n; ++i)
	{
		auto const& glyph = get_glyph(result, glyph_index{glyph_info[i].codepoint});
		auto const x_offset  = -glyph_pos[i].x_offset;
		auto const y_offset  = glyph_pos[i].y_offset;

		render(
			glyph,
			ret.pixels(),
			static_cast<uint32_t>((cursor_x + x_offset)/64) + glyph.x_offset,
			static_cast<uint32_t>((cursor_y + y_offset)/64) + static_cast<int32_t>(ascender) - glyph.y_offset
		);
		cursor_x += glyph_pos[i].x_advance;
		cursor_y -= glyph_pos[i].y_advance;
	}

	return ret;
}