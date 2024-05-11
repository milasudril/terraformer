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

	enum class codepoint : FT_ULong{};

	struct glyph
	{
		basic_image<uint8_t> image;
		displacement render_offset;
		displacement cursor_advancement;
	};


	class glyph_table
	{
	public:
		glyph& insert(codepoint index, glyph&& glyph_to_insert);

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

	using font = flat_map<std::greater<>, int, glyph_table>;

	glyph extract_glyph(FT_GlyphSlotRec const& ft_glyph);

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
			m_current_font_size = size;
			return *this;
		}

		auto& get_face()
		{ return m_face; }

		auto& get_glyph(codepoint charcode) const
		{
			assert(m_current_glyph_table != nullptr);

			auto const ret = m_current_glyph_table->find(charcode);
			if(ret != nullptr) [[likely]]
			{ return *ret; }

			return std::as_const(m_current_glyph_table->insert(charcode, load_glyph(charcode)));
		}

	private:
		glyph load_glyph(codepoint charcode) const;

		static thread_local font_loader m_loader;
		int m_current_font_size{0};
		glyph_table* m_current_glyph_table{nullptr};
		mutable font m_loaded_glyphs;
		mutable FT_Face m_face{};
	};

	inline thread_local font_loader glyph_renderer::m_loader{};
};

#endif