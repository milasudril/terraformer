#ifndef TERRAFORMER_UI_WIDGETS_BUTTON_HPP
#define TERRAFORMER_UI_WIDGETS_BUTTON_HPP

#include "ui/drawing_api/single_quad_renderer.hpp"
#include "ui/theming/widget_look.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"

namespace terraformer::ui::widgets
{
	class button
	{
	public:
		void prepare_for_presentation(
			drawing_api::single_quad_renderer::input_rectangle& output_rect,
			main::widget_instance_info const&,
			object_dict const& render_resources
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

				auto const tints = render_resources/"ui"/"misc_dark_colors";
				auto const num_colors = std::size(tints);
				assert(num_colors != 0);

				{
					auto const color = *(tints/((m_current_color + num_colors)%num_colors)).get_if<rgba_pixel const>();
					for(uint32_t y = 0; y != h; ++y)
					{
						for(uint32_t x = 0; x != w; ++x)
						{ img(x, y) = color; }
					}
					m_background.upload(std::as_const(img).pixels(), descriptor.num_mipmaps);
				}

				m_dirty = false;
			}

			output_rect.foreground = &m_foreground;
			output_rect.background = &m_background;
			constexpr std::array tints{
				rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
				rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
				rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
				rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f}
			};
			output_rect.foreground_tints = tints;
			output_rect.background_tints = tints;
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
			}
			return false;
		}

		main::widget_size_constraints get_size_constraints() const
		{
			return main::widget_size_constraints{
				.width{
					.min = 64.0f,
					.max = std::numeric_limits<float>::infinity()
				},
				.height{
					.min = 16.0f,
					.max = std::numeric_limits<float>::infinity()
				},
				.aspect_ratio = std::nullopt
			};
		}

		void handle_event(wsapi::fb_size size)
		{
			printf("%p Size was updated to %d %d\n", this, size.width, size.height);

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
		}

		template<class StringType>
		button& text(StringType&& text)
		{
			m_text = std::forward<StringType>(text);
			m_dirty = true;
			return *this;
		}

	private:
		std::string m_text;

		drawing_api::gl_texture m_foreground;
		drawing_api::gl_texture m_background;
		size_t m_current_color = 0;
		bool m_cursor_above = false;
		bool m_dirty = false;
	};
}

#endif
