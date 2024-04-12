#ifndef TERRAFORMER_UI_WIDGETS_TESTWIDGET_HPP
#define TERRAFORMER_UI_WIDGETS_TESTWIDGET_HPP

#include "ui/drawing_api/gl_texture.hpp"

namespace terraformer::ui::widgets
{
	class testwidget
	{
	public:
		void render()
		{ }

		auto const& background() const
		{ return m_background; }

		auto const& foreground() const
		{ return m_foreground; }

		bool handle_event(wsapi::cursor_position)
		{ return false;}

		bool handle_event(wsapi::mouse_button_event const&)
		{ return false; }

		wsapi::fb_size handle_event(wsapi::fb_size size)
		{ return size; }

	private:
		drawing_api::gl_texture m_foreground;
		drawing_api::gl_texture m_background;
	};
}

#endif