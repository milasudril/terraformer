#ifndef TERRAFORMER_UI_WIDGETS_XSECTION_VIEW_HPP
#define TERRAFORMER_UI_WIDGETS_XSECTION_VIEW_HPP

#include "./colorbar.hpp"
#include "./xsection_image_view.hpp"
#include "./widget_group.hpp"
#include "ui/layouts/table.hpp"
#include "ui/value_maps/affine_value_map.hpp"

namespace terraformer::ui::widgets
{
	class xsection_view:private widget_group<layouts::table>
	{
	public:
		using widget_group::handle_event;
		using widget_group::prepare_for_presentation;
		using widget_group::theme_updated;
		using widget_group::get_children;
		using widget_group::get_layout;
		using widget_group::compute_size;
		using widget_group::confirm_size;

		template<class ValueMap, class ColorMap>
		explicit xsection_view(
			iterator_invalidation_handler_ref iihr,
			std::u8string_view quantity_name,
			ValueMap const& vm,
			ColorMap const& cm
		):
			widget_group{iihr, 2u, layouts::table::cell_order::row_major},
			m_img_view{vm, cm},
			m_colorbar{iihr, quantity_name, vm, cm}
		{
			append(std::ref(m_img_view), terraformer::ui::main::widget_geometry{});
			append(std::ref(m_colorbar), terraformer::ui::main::widget_geometry{});
			layout.params().no_outer_margin = true;
			layout.set_cell_size(0, layouts::table::cell_size::expand{});
			auto const attributes = get_attributes();
			attributes.widget_states()[attributes.element_indices().front()].maximized = true;
			is_transparent = true;
		}

		void show_image(span_2d<float const> pixels)
		{ m_img_view.show_image(pixels); }

		void set_physical_dimensions(float width, float height)
		{
			m_img_view.set_physical_dimensions(width, height);

			// FIXME: Range depends on orientation
			m_colorbar.set_value_map(value_maps::affine_value_map{0.0f, height});
		}

		void set_orientation(float theta)
		{ m_img_view.set_orientation(theta); }

	private:
		xsection_image_view m_img_view;
		labeled_colorbar m_colorbar;
	};
}

#endif