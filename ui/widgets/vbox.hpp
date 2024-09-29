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
			auto const widget_states = widgets.widget_states();

			auto const n = std::size(widgets);
			auto min_width = 0.0f;
			auto max_width = std::numeric_limits<float>::infinity();
			auto height = margin_y;


			for(auto k = widgets.first_element_index(); k != n; ++k)
			{
				if(!widget_states[k].collapsed) [[likely]]
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
					max_width = std::min(max_width, constraints.width.max());
					height += widget_geometries[k].size[1] + margin_y;
				}
			}

			return main::widget_size_constraints{
				.width = main::widget_size_range{
					min_width + 2.0f*margin_x,
					std::max(min_width, max_width) + 2.0f*margin_x
				},
				.height = main::widget_size_range{
					height,
					std::numeric_limits<float>::infinity()
				},
				.aspect_ratio = std::nullopt
			};
		}

		float margin_x;
		float margin_y;
	};

	class vbox:public main::widget_with_default_actions
	{
	public:
		using widget_with_default_actions::handle_event;
		using widget_with_default_actions::compute_size_constraints;

		template<class Widget>
		vbox& append(Widget&& widget)
		{
			m_widgets.append(std::forward<Widget>(widget), terraformer::ui::main::widget_geometry{});
			return *this;
		}

 		void prepare_for_presentation(main::widget_rendering_result output_rect)
		{
			output_rect.set_background(m_background.get());
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
			output_rect.set_foreground(m_foreground.get());
		}

		void theme_updated(main::config const& new_theme, main::widget_instance_info instance_info)
		{
			auto const& panel = instance_info.section_level%2 == 0?
				new_theme.main_panel :
				new_theme.other_panel;
			layout.margin_x = panel.padding;
			layout.margin_y = panel.padding;
			m_background = panel.background_texture;
			m_background_tint = panel.colors.background;
			m_foreground = new_theme.misc_textures.null;
		}

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

		main::generic_shared_texture m_background;
		main::generic_shared_texture m_foreground;
		rgba_pixel m_background_tint;
	};
}

#endif
