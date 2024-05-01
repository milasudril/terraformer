#ifndef TERRAFORMER_UI_LAYOUT_HANDLING_WORKSPACE_HPP
#define TERRAFORMER_UI_LAYOUT_HANDLING_WORKSPACE_HPP

#include "ui/main/widget_list.hpp"

#include <functional>

namespace terraformer::ui::widgets
{
	template<class ... Args>
	void do_show_widgets(Args&&... args)
	{ show_widgets<0>(std::forward<Args>(args)...); }

	template<class TextureRepo, class WidgetRenderingResult>
	class workspace
	{
	public:
		using output_rectangle = WidgetRenderingResult;
		using texture_repo = TextureRepo;

		template<class ... Args>
		workspace& append(Args&&... args)
		{
			m_widgets.append(std::forward<Args>(args)...);
			return *this;
		}

		void render(
			output_rectangle& output_rect,
			texture_repo const& textures,
			theming::widget_look const& look
		)
		{
			// TODO: We should more conceptual names rather than something that describes the
			//       concrete texture
			output_rect.background = &textures.main_panel_background;
			output_rect.foreground = &textures.none;

			output_rect.foreground_tints = std::array{
				rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
				rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
				rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
				rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f}
			};

			output_rect.background_tints = std::array{
				look.colors.main_panel.background,
				look.colors.main_panel.background,
				look.colors.main_panel.background,
				look.colors.main_panel.background
			};
			render_widgets<0>(m_widgets, textures, look);
		}

		void handle_event(wsapi::cursor_enter_leave_event const&)
		{ }

		bool handle_event(wsapi::cursor_motion_event const& event)
		{
			// TODO: event.where must be converted to widget coordinates

			auto const i = find(event.where, m_widgets);
			auto const old_index = m_cursor_widget_index;
			m_cursor_widget_index = i;

			auto const widgets = m_widgets.widget_pointers();
			auto const cele_handlers = m_widgets.cursor_enter_leave_callbacks();
			if(i != old_index && old_index != widget_list::npos)
			{
				cele_handlers[old_index](
					widgets[old_index],
					wsapi::cursor_enter_leave_event{
						.where = event.where,
						.direction = wsapi::cursor_enter_leave::leave
					}
				);
			}

			if(i == widget_list::npos)
			{ return false; }

			if(i != old_index)
			{
				cele_handlers[i](
					widgets[i],
					wsapi::cursor_enter_leave_event{
						.where = event.where,
						.direction = wsapi::cursor_enter_leave::enter
					}
				);
			}

 			auto const cme_handlers = m_widgets.cursor_motion_callbacks();

			return cme_handlers[i](widgets[i], event);
		}

		bool handle_event(wsapi::mouse_button_event const& event)
		{
			// TODO: event.where must be converted to widget coordinates

			auto const i = find(event.where, m_widgets);
			if(i == widget_list::npos)
			{ return false; }

			auto const widgets = m_widgets.widget_pointers();
			auto const mbe_handlers = m_widgets.mouse_button_callbacks();

			return mbe_handlers[i](widgets[i], event);
		}

		wsapi::fb_size handle_event(wsapi::fb_size size)
		{
			auto const size_callbacks = m_widgets.size_callbacks();
			auto const widget_pointers = m_widgets.widget_pointers();
			auto const widget_visibilities = m_widgets.widget_visibilities();
			auto const widget_geometries = m_widgets.widget_geometries();
			auto const n = std::size(m_widgets);
			for(auto k = m_widgets.first_element_index(); k != n; ++k)
			{
				// TODO: Maybe size needs to be propagated to invisible widgets
				if(widget_visibilities[k] == main::widget_visibility::visible) [[likely]]
				{
					size_callbacks[k](
						widget_pointers[k],
						wsapi::fb_size {
							.width = static_cast<int>(widget_geometries[k].size[0]),
							.height = static_cast<int>(widget_geometries[k].size[1])
						}
					);
				}
			}
			return size;
		}

		template<class Renderer>
		void show_widgets(Renderer&& renderer)
		{ do_show_widgets(std::forward<Renderer>(renderer), m_widgets); }

	private:
		using widget_list = main::widget_list<TextureRepo, WidgetRenderingResult>;

		widget_list m_widgets;
		widget_list::index_type m_cursor_widget_index{widget_list::npos};
	};
}

#endif
