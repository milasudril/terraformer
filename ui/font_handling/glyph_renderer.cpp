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
		.render_offset = displacement{
			static_cast<float>(ft_glyph.bitmap_left),
			static_cast<float>(ft_glyph.bitmap_top),
			0.0f
		},
		.cursor_advancement = displacement{
			static_cast<float>(ft_glyph.advance.x)/64.0f,
			static_cast<float>(ft_glyph.advance.y)/64.0f,
			0.0f
		}
 	};

	for(uint32_t y = 0; y != ret.image.height(); ++y)
	{
		for(uint32_t x = 0; x != ret.image.width(); ++x)
		{ ret.image(x, y) = ft_glyph.bitmap.buffer[y * ft_glyph.bitmap.width + x]; }
	}

	return ret;
}

terraformer::ui::font_handling::glyph& terraformer::ui::font_handling::glyph_table::insert(
	codepoint index,
	glyph&& new_glyph
)
{
	if(static_cast<FT_ULong>(index) < std::size(m_latin_1))
	{ return m_latin_1[static_cast<FT_ULong>(index)] = std::move(new_glyph); }

	auto const i = m_other.insert(
		std::pair{
			std::as_const(index),
			std::move(new_glyph)
		}
	);

	return i.first->second;
}

terraformer::ui::font_handling::glyph
terraformer::ui::font_handling::glyph_renderer::load_glyph(codepoint charcode) const
{
	{
		auto const res = FT_Set_Pixel_Sizes(m_face, 0, m_current_font_size);
		if(res != FT_Err_Ok)
		{ throw std::runtime_error{get_ft_error_message(res)}; }
	}

	{
		auto const res = FT_Load_Char(m_face, static_cast<FT_ULong>(charcode), FT_LOAD_RENDER);
		if(res != FT_Err_Ok)
		{ throw std::runtime_error{get_ft_error_message(res)}; }
	}

	return extract_glyph(*m_face->glyph);
}