//@	{"target":{"name":"glyph_renderer.o"}}

#include "./glyph_renderer.hpp"

char const* terraformer::ui::font_handling::get_ft_error_message(FT_Error err)
{
	#undef FTERRORS_H_
	#define FT_ERRORDEF(error_code, value, string) case error_code: return string;
	#define FT_ERROR_START_LIST switch(err) {
	#define FT_ERROR_END_LIST default: return "Unknown error"; }
	#include FT_ERRORS_H
}

terraformer::ui::font_handling::glyph
terraformer::ui::font_handling::extract_glyph(FT_GlyphSlotRec const& ft_glyph)
{
	glyph ret{
		.image = basic_image<uint8_t>{
			ft_glyph.bitmap.width,
			ft_glyph.bitmap.rows
		},
		.x_offset = ft_glyph.bitmap_left,
		.y_offset = ft_glyph.bitmap_top
 	};

	for(uint32_t y = 0; y != ret.image.height(); ++y)
	{
		for(uint32_t x = 0; x != ret.image.width(); ++x)
		{ ret.image(x, y) = ft_glyph.bitmap.buffer[y * ft_glyph.bitmap.width + x]; }
	}

	return ret;
}

void terraformer::ui::font_handling::render(glyph const& glyph, span_2d<uint8_t> output_image, uint32_t x_offset, uint32_t y_offset)
{
	auto const img = std::as_const(glyph.image).pixels();
	for(uint32_t y = 0; y != img.height(); ++y)
	{
		for(uint32_t x = 0; x != img.width(); ++x)
		{
			auto const x_loc = std::min(x + x_offset, output_image.width() - 1);
			auto const y_loc = std::min(y + y_offset, output_image.height() - 1);
			auto const oldval = output_image(x_loc, y_loc);
			auto const input = static_cast<float>(img(x, y))/255.0f;
			auto const tmp = 2.0f*input*(1.0f - 0.5f*input);
			auto const output = static_cast<uint8_t>(tmp*255.0f);
			output_image(x_loc, y_loc) = std::max(oldval, output);
		}
	}
}

terraformer::ui::font_handling::glyph
terraformer::ui::font_handling::glyph_renderer::load_glyph(glyph_index index) const
{
	{
		auto const res = FT_Load_Glyph(m_face, static_cast<FT_UInt>(index), FT_LOAD_RENDER | FT_LOAD_TARGET_LIGHT);
		if(res != FT_Err_Ok)
		{ throw std::runtime_error{get_ft_error_message(res)}; }
	}

	return extract_glyph(*m_face->glyph);
}