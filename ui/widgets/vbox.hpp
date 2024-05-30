#ifndef TERRAFORMER_UI_WIDGETS_VBOX_HPP
#define TERRAFORMER_UI_WIDGETS_VBOX_HPP

#include "ui/main/widget_list.hpp"
#include "lib/common/resource_table.hpp"

#include <functional>

namespace terraformer::ui::widgets
{
	template<class TextureRepo, class WidgetRenderingResult>
	class vbox
	{
	public:
		using output_rectangle = WidgetRenderingResult;
		using texture_repo = TextureRepo;

		template<class Widget>
		vbox& append(Widget&& widget)
		{
			m_widgets.append(std::forward<Widget>(widget), terraformer::ui::main::widget_geometry{});
			return *this;
		}

 		void prepare_for_presentation(WidgetRenderingResult& output_rect, resource_table const& resources)
		{
			output_rect.background = resources.get_if("ui/main_panel/background_texture");
			output_rect.foreground = resources.get_if("ui/null_texture");
			output_rect.foreground_tints = std::array{
				rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
				rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
				rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
				rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f}
			};

			auto const background_color_ptr = resources.get_if<rgba_pixel>("ui/main_panel/background_tint");
			auto const background_color = background_color_ptr != nullptr?
				*background_color_ptr : rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f};

			output_rect.background_tints = std::array{
				background_color,
				background_color,
				background_color,
				background_color,
			};

 			prepare_for_presentation<0>(m_widgets, resources);
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

		main::widget_size_constraints const& get_size_constraints() const
		{ return m_current_size_constraints; }

		wsapi::fb_size handle_event(wsapi::fb_size size)
		{
			// TODO: The available size should not include any decorations (frames, scrollbars etc)
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
		{
			using main::show_widgets;
			show_widgets<0>(std::forward<Renderer>(renderer), m_widgets);
		}


		// Widget collection stuff

		void update_layout(float margin_x, float margin_y)
		{
			auto const widget_pointers = m_widgets.widget_pointers();
			auto const widget_geometries = m_widgets.widget_geometries();
			auto const size_constraints_callbacks = m_widgets.size_constraints_callbacks();
			auto const widget_visibilities = m_widgets.widget_visibilities();
			auto const n = std::size(m_widgets);
			auto min_width = 0.0f;
			auto max_width = std::numeric_limits<float>::infinity();
			auto height = margin_y;
			for(auto k = m_widgets.first_element_index(); k != n; ++k)
			{
				if(widget_visibilities[k] == main::widget_visibility::visible) [[likely]]
				{
					auto const constraints = size_constraints_callbacks[k](widget_pointers[k]);
					widget_geometries[k].where = location{
						margin_x,
						-height,
						0.0f
					};
					widget_geometries[k].origin = terraformer::location{-1.0f, 1.0f, 0.0f};
					widget_geometries[k].size = minimize_height(constraints);
					min_width = std::max(min_width, widget_geometries[k].size[0]);
					max_width = std::min(max_width, constraints.width.max);
					height += widget_geometries[k].size[1] + 4.0f;
				}
			}

			m_current_size_constraints = main::widget_size_constraints{
				.width{
					.min = min_width + 2.0f*margin_x,
					.max = std::max(min_width, max_width) + 2.0f*margin_x
				},
				.height{
					.min = height,
					.max = std::numeric_limits<float>::infinity()
				},
				.aspect_ratio = std::nullopt
			};
		}

		template<class Renderer>
		void decorate_widgets(Renderer&&, texture_repo const&, theming::widget_look const&)
		{ }

	private:
		using widget_list = main::widget_list<TextureRepo, WidgetRenderingResult>;

		widget_list m_widgets;
		widget_list::index_type m_cursor_widget_index{widget_list::npos};
		main::widget_size_constraints m_current_size_constraints;
	};
}

#endif
