#ifndef TERRAFORMER_UI_WIDGETS_VBOX_HPP
#define TERRAFORMER_UI_WIDGETS_VBOX_HPP

#include "ui/main/widget_collection.hpp"

#include <functional>

namespace terraformer::ui::widgets
{
	struct vbox_layout
	{
		main::widget_size_constraints update_widget_locations(main::widget_collection_ref& widgets) const
		{
			auto const size_constraints = std::as_const(widgets).size_constraints();
			auto const widget_geometries = widgets.widget_geometries();
			auto const widget_visibilities = widgets.widget_visibilities();

			auto const n = std::size(widgets);
			auto min_width = 0.0f;
			auto max_width = std::numeric_limits<float>::infinity();
			auto height = margin_y;

			for(auto k = widgets.first_element_index(); k != n; ++k)
			{
				if(widget_visibilities[k] == main::widget_visibility::visible) [[likely]]
				{
					auto const& constraints = size_constraints[k];
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

		float margin_x;
		float margin_y;
	};

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

		void handle_event(main::cursor_enter_event const&)
		{ }
		
		void handle_event(main::cursor_leave_event const&)
		{ }

		void handle_event(main::cursor_motion_event const& )
		{ }

		void handle_event(main::mouse_button_event const&)
		{  }

		void theme_updated(object_dict const& new_theme)
		{
			static size_t odd_even = 0;
			auto const panel = new_theme/"ui"/"panels"/0;
			assert(!panel.is_null());

			{
				auto const ptr = static_cast<float const*>(panel/"margins"/"x");
				assert(ptr != nullptr);
				layout.margin_x = *ptr;
			}

			{
				auto const ptr = static_cast<float const*>(panel/"margins"/"y");
				assert(ptr != nullptr);
				layout.margin_y = *ptr;
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

		main::widget_size_constraints compute_size_constraints()
		{ return main::widget_size_constraints{}; }

		main::layout_policy_ref get_layout() const
		{ return main::layout_policy_ref{std::ref(layout)}; }

		main::widget_collection_ref get_children()
		{ return m_widgets.get_attributes(); }

		main::widget_collection_view get_children() const
		{ return m_widgets.get_attributes(); }

	private:
		using widget_collection = main::widget_collection;

		widget_collection m_widgets;
		widget_collection::index_type m_cursor_widget_index{widget_collection::npos};
		vbox_layout layout;

		shared_const_any m_background;
		shared_const_any m_foreground;
		rgba_pixel m_background_tint;
	};
}

#endif
