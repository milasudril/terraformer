//@	{
//@		"dependencies":[{"ref":"freetype2", "origin":"pkg-config"}],
//@		"dependencies_extra": [{"ref":"./glyph_renderer.o", "rel":"implementation"}]
//@	}

#ifndef TERRAFORMER_UI_FONT_HANDLING_GLYPH_RENDERER_HPP
#define TERRAFORMER_UI_FONT_HANDLING_GLYPH_RENDERER_HPP

#include "lib/array_classes/single_array.hpp"
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

	enum class codepoint : FT_ULong{};

	struct glyph
	{
		basic_image<uint8_t> image;
		displacement render_offset;
		displacement cursor_advancement;
	};

	glyph extract_glyph(FT_GlyphSlotRec const& ft_glyph);

	class glyph_table
	{
	public:
		glyph& insert(codepoint index, FT_GlyphSlotRec const& glyph);

		glyph const* find(codepoint index) const
		{
			if(static_cast<FT_ULong>(index) < 256) [[likely]]
			{
				auto const& slot = m_latin_1[static_cast<FT_ULong>(index)];
				return slot.image.has_pixels() ? &slot : nullptr;
			}

			auto const i = m_other.find(index);
			return i != std::end(m_other) ? &i->second : nullptr;
		}

	private:
		std::array<glyph, 256> m_latin_1;
		std::unordered_map<codepoint, glyph> m_other;
	};

	using font = single_array<glyph_table>;
	using font_size = font::index_type;

	class glyph_renderer
	{
	public:
		glyph_renderer():m_face{nullptr}{}

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

		auto& get_glyph(font_size size, codepoint charcode) const
		{
			assert(size.get() != 0);

			auto const font_size_index = size - 1;
			if(font_size_index < std::size(m_loaded_glyphs)) [[likely]]
			{
				auto const ret = m_loaded_glyphs[font_size_index].find(charcode);
				if(ret != nullptr) [[likely]]
				{ return *ret; }
			}

			return std::as_const(load_glyph(size, charcode));
		}

	private:
		glyph& load_glyph(font_size size, codepoint charcode) const;

		static thread_local font_loader m_loader;
		mutable font m_loaded_glyphs;
		mutable font_size m_active_font_size;
		mutable FT_Face m_face;
	};

	inline thread_local font_loader glyph_renderer::m_loader{};
};

#endif