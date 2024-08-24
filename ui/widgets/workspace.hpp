#ifndef TERRAFORMER_UI_LAYOUT_HANDLING_WORKSPACE_HPP
#define TERRAFORMER_UI_LAYOUT_HANDLING_WORKSPACE_HPP

#include "ui/main/widget_collection.hpp"

#include <functional>

namespace terraformer::ui::widgets
{
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

		void handle_event(main::cursor_enter_leave_event const&)
		{ }

		void handle_event(main::cursor_motion_event const& event)
		{
			// TODO: event.where must be converted to widget coordinates
			// TODO: Separate frame from the widget itself

			auto const i = find(event.where, m_widgets);
			auto const old_index = m_cursor_widget_index;
			m_cursor_widget_index = i;

			auto const children = m_widgets.get_attributes();
			auto const widgets = children.widget_pointers();
			auto const cele_handlers = children.cursor_enter_leave_callbacks();
			if(i != old_index && old_index != widget_collection::npos)
			{
				cele_handlers[old_index](
					widgets[old_index],
					main::cursor_enter_leave_event{
						.where = event.where,
						.direction = main::cursor_enter_leave::leave
					}
				);
			}

			if(i == widget_collection::npos)
			{ return; }

			if(i != old_index)
			{
				cele_handlers[i](
					widgets[i],
					main::cursor_enter_leave_event{
						.where = event.where,
						.direction = main::cursor_enter_leave::enter
					}
				);
			}

 			auto const cme_handlers = children.cursor_motion_callbacks();

			cme_handlers[i](widgets[i], event);
		}

		void handle_event(main::mouse_button_event const& event)
		{
			// TODO: event.where must be converted to widget coordinates
			// TODO: Separate frame from the widget itself
			auto const children = std::as_const(m_widgets).get_attributes();
			auto const i = find(event.where, children);
			if(i == widget_collection::npos)
			{ return; }

			auto const widgets = children.widget_pointers();
			auto const mbe_handlers = children.mouse_button_callbacks();

			mbe_handlers[i](widgets[i], event);
		}

		main::fb_size handle_event(main::fb_size size)
		{
			// TODO: The available size should not include any decorations (frames, scrollbars etc)
			auto const children = m_widgets.get_attributes();
			auto const size_callbacks = children.size_callbacks();
			auto const widget_pointers = children.widget_pointers();
			auto const widget_visibilities = children.widget_visibilities();
			auto const widget_geometries = children.widget_geometries();
			auto const n = std::size(children);
			for(auto k = children.first_element_index(); k != n; ++k)
			{
				// TODO: Maybe size needs to be propagated to invisible widgets
				if(widget_visibilities[k] == main::widget_visibility::visible) [[likely]]
				{
					size_callbacks[k](
						widget_pointers[k],
						main::fb_size {
							.width = static_cast<int>(widget_geometries[k].size[0]),
							.height = static_cast<int>(widget_geometries[k].size[1])
						}
					);
				}
			}
			return size;
		}

	private:
		using widget_collection = main::widget_collection;

		widget_collection m_widgets;
		widget_collection::index_type m_cursor_widget_index{widget_collection::npos};
	};
}

#endif
