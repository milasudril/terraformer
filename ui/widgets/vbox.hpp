#ifndef TERRAFORMER_UI_WIDGETS_VBOX_HPP
#define TERRAFORMER_UI_WIDGETS_VBOX_HPP

#include "ui/layouts/rowmajor_table.hpp"

#include "ui/main/widget_collection.hpp"

#include <functional>

namespace terraformer::ui::widgets
{
	class vbox:public main::widget_with_default_actions
	{
	public:
		using widget_with_default_actions::handle_event;
		using widget_with_default_actions::compute_size;

		template<class Widget>
		vbox& append(Widget&& widget)
		{
			m_widgets.append(std::forward<Widget>(widget), terraformer::ui::main::widget_geometry{});
			return *this;
		}

 		void prepare_for_presentation(main::widget_rendering_result output_rect)
		{
			std::array const bg_tints{
				m_background_tint,
				m_background_tint,
				m_background_tint,
				m_background_tint,
			};

			output_rect.set_widget_background(m_background.get(), bg_tints);
			output_rect.set_bg_layer_mask(m_null_texture.get());
			output_rect.set_selection_background(m_null_texture.get(), std::array<rgba_pixel, 4>{});
			output_rect.set_widget_foreground(m_null_texture.get(), std::array<rgba_pixel, 4>{}, displacement{});
			output_rect.set_frame(m_null_texture.get(), std::array<rgba_pixel, 4>{});
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
			m_null_texture = new_theme.misc_textures.null;
		}

		main::layout_policy_ref get_layout()
		{ return main::layout_policy_ref{std::ref(layout)}; }

		main::widget_collection_ref get_children()
		{ return m_widgets.get_attributes(); }

		main::widget_collection_view get_children() const
		{ return m_widgets.get_attributes(); }

	private:
		using widget_collection = main::widget_collection;

		widget_collection m_widgets;
		widget_collection::index_type m_cursor_widget_index{widget_collection::npos};
		layouts::rowmajor_table layout{2};

		main::generic_shared_texture m_background;
		main::generic_shared_texture m_null_texture;
		rgba_pixel m_background_tint;
	};
}

#endif
