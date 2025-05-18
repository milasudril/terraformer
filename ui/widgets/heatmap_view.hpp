#ifndef TERRAFORMER_UI_WIDGETS_HEATMAP_VIEW_HPP
#define TERRAFORMER_UI_WIDGETS_HEATMAP_VIEW_HPP

#include "./colorbar.hpp"
#include "./false_color_image_view.hpp"
#include "./widget_group.hpp"
#include "ui/layouts/table.hpp"

namespace terraformer::ui::widgets
{
	class heatmap_view:private widget_group<layouts::table>
	{
	public:
		using widget_group::handle_event;
		using widget_group::prepare_for_presentation;
		using widget_group::theme_updated;
		using widget_group::get_children;
		using widget_group::get_layout;
		using widget_group::compute_size;
		using widget_group::confirm_size;

		explicit float_input(iterator_invalidation_handler_ref iihr):
			widget_group{iihr, 2u, layouts::table::cell_order::row_major}
		{ init(); }


	private:
		false_color_image_view m_img_view;
		colorbar m_color_bar;

		main::widget_user_interaction_handler<float_input> m_on_value_changed{no_operation_tag{}};

		void init()
		{
			append(std::ref(m_img_view), terraformer::ui::main::widget_geometry{});
			append(std::ref(m_colorbar), terraformer::ui::main::widget_geometry{});
			m_input_widget.on_value_changed([this]<class ... Args>(auto const& input, Args&&... args){
				m_textbox.value(input.value());
				m_on_value_changed(*this, std::forward<Args>(args)...);
			});
			layout.params().no_outer_margin = true;
			is_transparent = true;
		}
}

#endif