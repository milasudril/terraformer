//@	{
//@		"dependencies":[
//@			{"ref":"freetype2", "origin":"pkg-config"}
//@		]
//@	}

#ifndef TERRAFORMER_UI_FONT_HANDLING_GLYPH_RENDERER_HPP
#define TERRAFORMER_UI_FONT_HANDLING_GLYPH_RENDERER_HPP

#include <ft2build.h>
#include FT_FREETYPE_H

namespace terraformer::ui::font_handling
{
	class glyph_renderer
	{
	public:
		explicit glyph_renderer()
		{
			if(m_ft_usecount == 0) [[unlikely]]
			{	FT_Init_FreeType(&m_font_loader); }
			++m_ft_usecount;
		}

		~glyph_renderer()
		{
			--m_ft_usecount;
			if(m_ft_usecount == 0)
			{ FT_Done_FreeType(m_font_loader); }
		}

		static size_t loader_usecount()
		{ return m_ft_usecount; }

	private:
		static thread_local size_t m_ft_usecount;
		static thread_local FT_Library m_font_loader;
	};

	thread_local size_t glyph_renderer::m_ft_usecount{};
	thread_local FT_Library glyph_renderer::m_font_loader{};
};

#endif