#ifndef TERRAFORMER_UI_WIDGETS_VBOX_HPP
#define TERRAFORMER_UI_WIDGETS_VBOX_HPP

#include "ui/main/widget_collection.hpp"

#include <functional>

namespace terraformer::ui::widgets
{
	class vbox
	{
	public:
		template<class Widget>
		vbox& append(Widget&& widget)
		{
			m_widgets.append(std::forward<Widget>(widget), terraformer::ui::main::widget_geometry{});
			return *this;
		}

 		void prepare_for_presentation(main::widget_rendering_result output_rect)
		{
			output_rect.set_foreground(m_foreground.get_if<main::generic_unique_texture const>()->get());
			output_rect.set_background(m_background.get_if<main::generic_unique_texture const>()->get());
			output_rect.set_foreground_tints(std::array{
				rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
				rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
				rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
				rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f}
			});
			output_rect.set_background_tints(std::array{
				m_background_tint,
				m_background_tint,
				m_background_tint,
				m_background_tint,
			});
		}

		void handle_event(main::cursor_enter_leave_event const&)
		{ }

		template<class WidgetRenderingResult>
		main::widgets_to_render_list<WidgetRenderingResult> collect_widgets_to_render() const
		{
			return main::widgets_to_render_list<WidgetRenderingResult>{m_widgets};
		}

		void handle_event(main::cursor_motion_event const& event)
		{
			// TODO: event.where must be converted to widget coordinates

			auto const i = find(event.where, m_widgets);
			auto const old_index = m_cursor_widget_index;
			m_cursor_widget_index = i;

			auto const children = get_children();
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

			return cme_handlers[i](widgets[i], event);
		}

		void handle_event(main::mouse_button_event const& event)
		{
			// TODO: event.where must be converted to widget coordinates
			auto const i = find(event.where, std::as_const(*this).get_children());
			if(i == widget_collection::npos)
			{ return; }

			auto const children = get_children();
			auto const widgets = children.widget_pointers();
			auto const mbe_handlers = children.mouse_button_callbacks();

			return mbe_handlers[i](widgets[i], event);
		}

		void theme_updated(object_dict const& new_theme)
		{
			static size_t odd_even = 0;
			auto const panel = new_theme/"ui"/"panels"/0;
			assert(!panel.is_null());

			{
				auto const ptr = static_cast<float const*>(panel/"margins"/"x");
				assert(ptr != nullptr);
				m_margin_x = *ptr;
			}
			{
				auto const ptr = static_cast<float const*>(panel/"margins"/"y");
				assert(ptr != nullptr);
				m_margin_y = *ptr;
			}

			m_background = panel.dup("background_texture");
			m_foreground = (new_theme/"ui").dup("null_texture");

			auto const background_color_ptr = (panel/"background_tint").get_if<rgba_pixel const>();
			m_background_tint = background_color_ptr != nullptr?
				*background_color_ptr : rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f};

			using main::theme_updated;
			theme_updated(m_widgets, new_theme);


			m_background_tint *= (odd_even%2)? 0.75f : 1.0f;
			++odd_even;
		}

		void handle_event(main::fb_size)
		{ }

		main::widget_size_constraints update_geometry()
		{
			auto const margin_x = m_margin_x;
			auto const margin_y = m_margin_y;
			auto const children = get_children();
			auto const widget_pointers = children.widget_pointers();
			auto const widget_geometries = children.widget_geometries();
			auto const update_geometries = children.update_geometry_callbacks();
			auto const widget_visibilities = children.widget_visibilities();
			auto const size_callbacks = children.size_callbacks();
			auto const n = std::size(children);
			auto min_width = 0.0f;
			auto max_width = std::numeric_limits<float>::infinity();
			auto height = margin_y;
			for(auto k = children.first_element_index(); k != n; ++k)
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

		main::layout_policy_ref get_layout() const
		{ return main::layout_policy_ref{}; }

		main::widget_collection_ref get_children()
		{ return m_widgets.get_attributes(); }

		main::widget_collection_view get_children() const
		{ return m_widgets.get_attributes(); }

	private:
		using widget_collection = main::widget_collection;

		widget_collection m_widgets;
		widget_collection::index_type m_cursor_widget_index{widget_collection::npos};
		float m_margin_x;
		float m_margin_y;

		shared_const_any m_background;
		shared_const_any m_foreground;
		rgba_pixel m_background_tint;

	};
}

#endif
