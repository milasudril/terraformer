#ifndef TERRAFORMER_UI_WIDGETS_TESTWIDGET_HPP
#define TERRAFORMER_UI_WIDGETS_TESTWIDGET_HPP

#include "ui/drawing_api/gl_texture.hpp"
#include "ui/theming/color_scheme.hpp"

namespace terraformer::ui::widgets
{
	class testwidget
	{
	public:
		void render()
		{
			if(m_dirty)
			{
				printf("Rendering\n");
				m_dirty = false;
			}
		}

		auto const& background() const
		{ return m_background; }

		auto const& foreground() const
		{ return m_foreground; }

		bool handle_event(wsapi::cursor_position)
		{ return false;}

		bool handle_event(wsapi::mouse_button_event const& mbe)
		{
			auto const num_colors = std::size(theming::current_color_scheme.misc_dark_colors);

			if((mbe.button == 0 || mbe.button == 1)
				&& mbe.action == wsapi::button_action::release)
			{
				auto const dir = mbe.button == 0 ? -1 : 1;
				m_current_color = (m_current_color + dir + num_colors)%num_colors;
				m_dirty = true;
				printf("Hej %zu\n", m_current_color);
			}
			return false;
		}

		wsapi::fb_size handle_event(wsapi::fb_size size)
		{	return size; }

	private:
		drawing_api::gl_texture m_foreground;
		drawing_api::gl_texture m_background;
		size_t m_current_color = 0;
		bool m_dirty = false;
	};
}

#endif