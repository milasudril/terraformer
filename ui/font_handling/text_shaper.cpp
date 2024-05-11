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
		width += (i != n - 1)? glyph_pos[i].x_advance : glyph.image.width()*64;
		height += (i != n - 1)? glyph_pos[i].y_advance : glyph.image.height()*64;
	}

	auto const& renderer = result.renderer.get();

	height = std::max(static_cast<uint64_t>(renderer.get_global_glyph_height()), height);
	width = std::max(static_cast<uint64_t>(renderer.get_global_glyph_width()), width);

	return span_2d_extents{
		.width = narrowing_cast<uint32_t>(width/64),
		.height = narrowing_cast<uint32_t>(height/64)
	};
}
#if 0
namespace
{
	void dump_glyph(terraformer::span_2d<uint8_t const> img)
	{
		for(uint32_t y = 0; y != img.height(); ++y)
		{
			for(uint32_t x = 0; x != img.width(); ++x)
			{
				if(img(x, y) > 128)
				{ putchar('*'); }
				else
				{ putchar('-'); }
			}
			putchar('\n');
		}
		putchar('\n');
	}
}
#endif

terraformer::basic_image<uint8_t>
terraformer::ui::font_handling::render(shaping_result const& result)
{
	auto const size = compute_extents(result);
	terraformer::basic_image<uint8_t> ret{size.width, size.height};

	auto const n = result.glyph_count;
	auto const glyph_info = result.glyph_info;
	auto const glyph_pos = result.glyph_pos;

	uint64_t x_offset = 0;
	uint64_t y_offset = 0;

	for(size_t i = 0; i != n; ++i)
	{
		auto const& glyph = get_glyph(result, glyph_index{glyph_info[i].codepoint});
		render(glyph, ret.pixels(), static_cast<uint32_t>(x_offset/64), static_cast<uint32_t>(y_offset/64));
		x_offset += glyph_pos[i].x_advance;
		y_offset += glyph_pos[i].y_advance;
	}

	return ret;
}