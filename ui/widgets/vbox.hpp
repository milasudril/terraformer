#ifndef TERRAFORMER_UI_WIDGETS_VBOX_HPP
#define TERRAFORMER_UI_WIDGETS_VBOX_HPP

#include "ui/main/widget_list.hpp"

#include <functional>

namespace terraformer::ui::widgets
{
	template<class WidgetRenderingResult>
	class vbox
	{
	public:
		using output_rectangle = WidgetRenderingResult;

		template<class Widget>
		vbox& append(Widget&& widget)
		{
			m_widgets.append(std::forward<Widget>(widget), terraformer::ui::main::widget_geometry{});
			return *this;
		}

 		void prepare_for_presentation(
			WidgetRenderingResult& output_rect,
			main::widget_instance_info const& instance_info,
			object_dict const& resources
		)
		{
			auto const panel = resources/"ui"/"panels"/0;
			output_rect.background = panel/"background_texture";
			output_rect.foreground = resources/"ui"/"null_texture";
			output_rect.foreground_tints = std::array{
				rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
				rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
				rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
				rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f}
			};

			auto const background_color_ptr = (panel/"background_tint").get_if<rgba_pixel const>();
			auto const background_color = background_color_ptr != nullptr?
				*background_color_ptr : rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f};

			output_rect.background_tints = std::array{
				background_color,
				background_color,
				background_color,
				background_color,
			};

 			prepare_widgets_for_presentation(
				m_widgets,
				main::widget_instance_info{
					.section_level = instance_info.section_level,
					.paragraph_index = 0
				},
				resources
			);
		}

		void handle_event(main::cursor_enter_leave_event const&)
		{ }


		void handle_event(main::cursor_motion_event const& event)
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
					main::cursor_enter_leave_event{
						.where = event.where,
						.direction = main::cursor_enter_leave::leave
					}
				);
			}

			if(i == widget_list::npos)
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

 			auto const cme_handlers = m_widgets.cursor_motion_callbacks();

			return cme_handlers[i](widgets[i], event);
		}

		void handle_event(main::mouse_button_event const& event)
		{
			// TODO: event.where must be converted to widget coordinates
			auto const i = find(event.where, m_widgets);
			if(i == widget_list::npos)
			{ return; }

			auto const widgets = m_widgets.widget_pointers();
			auto const mbe_handlers = m_widgets.mouse_button_callbacks();

			return mbe_handlers[i](widgets[i], event);
		}

		void theme_updated(object_dict const& new_theme)
		{
			{
				auto ptr = static_cast<float const*>(new_theme/"ui"/"panels"/0/"margins"/"x");
				assert(ptr != nullptr);
				m_margin_x = *ptr;
			}
			{
				auto ptr = static_cast<float const*>(new_theme/"ui"/"panels"/0/"margins"/"y");
				assert(ptr != nullptr);
				m_margin_y = *ptr;
			}

			using main::theme_updated;
			theme_updated(m_widgets, new_theme);
		}

		main::fb_size handle_event(main::fb_size size)
		{
			return size;
		}

		template<class Renderer>
		void show_widgets(Renderer&& renderer)
		{
			using main::show_widgets;
			show_widgets(std::forward<Renderer>(renderer), m_widgets);
		}

		main::widget_size_constraints update_geometry()
		{
			auto const margin_x = m_margin_x;
			auto const margin_y = m_margin_y;
			auto const widget_pointers = m_widgets.widget_pointers();
			auto const widget_geometries = m_widgets.widget_geometries();
			auto const update_geometries = m_widgets.update_geometry_callbacks();
			auto const widget_visibilities = m_widgets.widget_visibilities();
			auto const size_callbacks = m_widgets.size_callbacks();
			auto const n = std::size(m_widgets);
			auto min_width = 0.0f;
			auto max_width = std::numeric_limits<float>::infinity();
			auto height = margin_y;
			for(auto k = m_widgets.first_element_index(); k != n; ++k)
			{
				if(widget_visibilities[k] == main::widget_visibility::visible) [[likely]]
				{
					auto const constraints = update_geometries[k](widget_pointers[k]);
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

					size_callbacks[k](
						widget_pointers[k],
						main::fb_size {
							.width = static_cast<int>(widget_geometries[k].size[0]),
							.height = static_cast<int>(widget_geometries[k].size[1])
						}
					);
				}
			}

			return main::widget_size_constraints{
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
		void decorate_widgets(
			Renderer&&,
			main::widget_instance_info const& instance_info,
			object_dict const& resources
		)
		{ }

	private:
		using widget_list = main::widget_list<WidgetRenderingResult>;

		widget_list m_widgets;
		widget_list::index_type m_cursor_widget_index{widget_list::npos};
		float m_margin_x;
		float m_margin_y;
	};
}

#endif
