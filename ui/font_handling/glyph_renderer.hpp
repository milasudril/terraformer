//@	{
//@		"dependencies":[{"ref":"freetype2", "origin":"pkg-config"}],
//@		"dependencies_extra": [{"ref":"./glyph_renderer.o", "rel":"implementation"}]
//@	}

#ifndef TERRAFORMER_UI_FONT_HANDLING_GLYPH_RENDERER_HPP
#define TERRAFORMER_UI_FONT_HANDLING_GLYPH_RENDERER_HPP

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdexcept>

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

		glyph_renderer& set_font_size(int new_size)
		{
			auto const res = FT_Set_Pixel_Sizes(m_face, 0, new_size);
			if(res != FT_Err_Ok)
			{ throw std::runtime_error{get_ft_error_message(res)}; }
			return *this;
		}

		auto& get_glyph(codepoint charcode) const
		{
			auto const index = FT_Get_Char_Index(m_face, static_cast<FT_ULong>(charcode));

			auto const res = FT_Load_Glyph(m_face, index, FT_LOAD_DEFAULT);
			if(res != FT_Err_Ok)
			{ throw std::runtime_error{get_ft_error_message(res)}; }

			if(m_face->glyph->format != FT_GLYPH_FORMAT_BITMAP) [[likely]]
			{
				auto const res = FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL);
				if(res != FT_Err_Ok)
				{ throw std::runtime_error{get_ft_error_message(res)}; }
			}

			return m_face->glyph;
		}

	private:
		static thread_local font_loader m_loader;
		mutable FT_Face m_face;
	};

	inline thread_local font_loader glyph_renderer::m_loader{};
};

#endif