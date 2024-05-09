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
	// TODO: Set copy pixels into image
	// TODO: Set correct values in render_offset
	// TODO: Set correct values in cursor_advancement

	return glyph{
		.image = basic_image<uint8_t>{
			ft_glyph.bitmap.width,
			ft_glyph.bitmap.rows
		},
		.render_offset{},
		.cursor_advancement{}
	};
}

terraformer::ui::font_handling::glyph& terraformer::ui::font_handling::glyph_table::insert(codepoint index, FT_GlyphSlotRec const& ft_glyph)
{
	if(static_cast<FT_ULong>(index) < std::size(m_latin_1))
	{ return m_latin_1[static_cast<FT_ULong>(index)] = extract_glyph(ft_glyph); }

	auto const i = m_other.insert(
		std::pair{
			std::as_const(index),
			extract_glyph(ft_glyph)
		}
	);

	return i.first->second;
}

terraformer::ui::font_handling::glyph&
terraformer::ui::font_handling::glyph_renderer::load_glyph(font_size size, codepoint charcode) const
{
	assert(size.get() != 0);

	auto const font_size_index = size - 1;
	if(size != m_active_font_size) [[unlikely]]
	{
		auto const res = FT_Set_Pixel_Sizes(m_face, 0, static_cast<int>(size.get()));
		if(res != FT_Err_Ok)
		{ throw std::runtime_error{get_ft_error_message(res)}; }
		m_active_font_size = size;
	}

	auto const res = FT_Load_Char(m_face, static_cast<FT_ULong>(charcode), FT_LOAD_RENDER);
	if(res != FT_Err_Ok)
	{ throw std::runtime_error{get_ft_error_message(res)}; }

	if(font_size_index.get() >= std::size(m_loaded_glyphs).get())
	{ m_loaded_glyphs.resize(static_cast<font::size_type>(size)); }

	return m_loaded_glyphs[font_size_index].insert(charcode, *m_face->glyph);
}