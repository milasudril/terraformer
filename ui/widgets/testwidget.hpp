#ifndef TERRAFORMER_UI_WIDGETS_TESTWIDGET_HPP
#define TERRAFORMER_UI_WIDGETS_TESTWIDGET_HPP

#include "ui/drawing_api/single_quad_renderer.hpp"
#include "ui/theming/widget_look.hpp"

namespace terraformer::ui::widgets
{
	class testwidget
	{
	public:
		void render(
			drawing_api::single_quad_renderer::input_rectangle& output_rect,
			theming::widget_look const& look
		)
		{
			if(m_dirty)
			{
				auto const& descriptor = m_foreground.descriptor();
				auto const w = static_cast<uint32_t>(descriptor.width);
				auto const h = static_cast<uint32_t>(descriptor.height);

				image img{w, h};
				for(uint32_t y = 0; y != h; ++y)
				{
					for(uint32_t x = 0; x != w; ++x)
					{ img(x, y) = rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f}; }
				}
				m_foreground.upload(std::as_const(img).pixels(), descriptor.num_mipmaps);

				auto const num_colors = std::size(look.colors.misc_bright_colors);
				{
					auto const color = look.colors.misc_bright_colors[(m_current_color + num_colors + 1)%num_colors];
					for(uint32_t x = 0; x != w; ++x)
					{
						img(x, 0) = color;
						img(x, 1) = color;
						img(x, 2) = color;
						img(x, 3) = color;
						img(x, h - 1) = color;
						img(x, h - 2) = color;
						img(x, h - 3) = color;
						img(x, h - 4) = color;
					}

					m_border.upload(std::as_const(img).pixels(), descriptor.num_mipmaps);
				}

				{
					auto const color = look.colors.misc_dark_colors[(m_current_color + num_colors)%num_colors];
					for(uint32_t y = 0; y != h; ++y)
					{
						for(uint32_t x = 0; x != w; ++x)
						{ img(x, y) = color; }
					}
					m_background.upload(std::as_const(img).pixels(), descriptor.num_mipmaps);
				}

				m_dirty = false;
			}
			output_rect.foreground = m_cursor_above? &m_border : &m_foreground;
			output_rect.background = &m_background;
		}

		void handle_event(wsapi::cursor_enter_leave_event const& cele)
		{
			m_cursor_above = cele.direction == wsapi::cursor_enter_leave::enter? true :false;
		}

		bool handle_event(wsapi::cursor_motion_event const&)
		{
			return false;
		}

		bool handle_event(wsapi::mouse_button_event const& mbe)
		{
			if((mbe.button == 0 || mbe.button == 1)
				&& mbe.action == wsapi::button_action::release)
			{
				auto const dir = mbe.button == 0 ? -1 : 1;
				m_current_color += dir;
				m_dirty = true;
				printf("Hej %zu\n", m_current_color);
			}
			return false;
		}

		wsapi::fb_size handle_event(wsapi::fb_size size)
		{
			printf("Size was updated to %d %d\n", size.width, size.height);

			drawing_api::gl_texture_descriptor descriptor{
				.width = size.width,
				.height = size.height,
				.format = drawing_api::to_gl_color_channel_layout_v<rgba_pixel>,
				.type = drawing_api::to_gl_type_id_v<rgba_pixel>,
				.num_mipmaps = 1
			};

			m_foreground.set_format(descriptor);
			m_background.set_format(descriptor);
			m_dirty = true;
			return size;

		}

	private:
		drawing_api::gl_texture m_foreground;
		drawing_api::gl_texture m_border;
		drawing_api::gl_texture m_background;
		size_t m_current_color = 0;
		bool m_cursor_above = false;
		bool m_dirty = false;
	};
}

#endif