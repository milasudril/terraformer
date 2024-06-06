//@	{
//@		"dependencies":[{"ref":"freetype2", "origin":"pkg-config"}],
//@		"dependencies_extra": [{"ref":"./glyph_renderer.o", "rel":"implementation"}]
//@	}

#ifndef TERRAFORMER_UI_FONT_HANDLING_GLYPH_RENDERER_HPP
#define TERRAFORMER_UI_FONT_HANDLING_GLYPH_RENDERER_HPP

#include "lib/common/flat_map.hpp"
#include "lib/pixel_store/image.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdexcept>
#include <cassert>

namespace terraformer::ui::font_handling
{
	char const* get_ft_error_message(FT_Error number);

	class font_loader
	{
	public:
		font_loader()
		{
			auto const res = FT_Init_FreeType(&m_handle);
			if(res != FT_Err_Ok)
			{ throw std::runtime_error{get_ft_error_message(res)}; }
		}

		~font_loader()
		{ FT_Done_FreeType(m_handle); }

		auto handle() const
		{ return m_handle; }

	private:
		FT_Library m_handle;
	};

	enum class glyph_index : FT_UInt {};

	enum class codepoint : FT_ULong {};

	struct glyph
	{
		basic_image<uint8_t> image;
		int32_t x_offset;
		int32_t y_offset;
	};

	glyph extract_glyph(FT_GlyphSlotRec const& ft_glyph);

	void render(glyph const& glyph, span_2d<uint8_t> output_image, uint32_t x_offset, uint32_t y_offset);

	class glyph_renderer
	{
	public:
		glyph_renderer():m_face{nullptr}
		{ m_loaded_glyphs.reserve(4); }

		explicit glyph_renderer(char const* filename)
		{
			auto const res = FT_New_Face(m_loader.handle(), filename, 0, &m_face);
			if(res != FT_Err_Ok)
			{ throw std::runtime_error{get_ft_error_message(res)}; }
		}

		~glyph_renderer()
		{
			if(m_face != nullptr)
			{ FT_Done_Face(m_face); }
		}

		auto& set_font_size(int size)
		{
			{
				auto const res = FT_Set_Pixel_Sizes(m_face, 0, size);
				if(res != FT_Err_Ok)
				{ throw std::runtime_error{get_ft_error_message(res)}; }
			}

			auto const res = m_loaded_glyphs.insert(size, glyph_table{});
			m_current_glyph_table = &m_loaded_glyphs.values<0>()[res.first];
			if(res.second)
			{ m_current_glyph_table->reserve(256); }

			m_current_font_size = size;
			return *this;
		}

		auto& get_face()
		{ return m_face; }

		auto& get_glyph(glyph_index index) const
		{
			assert(m_current_glyph_table != nullptr);

			auto const ret = m_current_glyph_table->find(index);
			if(ret != glyph_table::npos) [[likely]]
			{ return std::as_const(m_current_glyph_table->values<0>()[ret]); }

			auto const i = m_current_glyph_table->insert(index, load_glyph(index));
			return std::as_const(m_current_glyph_table->values<0>()[i.first]);
		}

		auto& get_glyph(codepoint charcode) const
		{
			return get_glyph(glyph_index{FT_Get_Char_Index(m_face, static_cast<FT_ULong>(charcode))});
		}

		auto get_global_glyph_height() const
		{ return m_face->ascender - m_face->descender; }

		auto get_ascender() const
		{ return m_face->ascender; }

		auto get_global_glyph_width() const
		{ return m_face->max_advance_width; }

	private:
		glyph load_glyph(glyph_index index) const;

		static thread_local font_loader m_loader;
		int m_current_font_size{0};
		using glyph_table = flat_map<std::less<>, glyph_index, glyph>;
		glyph_table* m_current_glyph_table{nullptr};
		mutable flat_map<
			std::greater<>,
			int,
			glyph_table
		> m_loaded_glyphs;
		FT_Face m_face{};
	};

	inline thread_local font_loader glyph_renderer::m_loader{};
};

#endif