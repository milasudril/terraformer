//@	{"dependencies_extra":[{"ref":"./colorbar.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_COLORBAR_HPP
#define TERRAFORMER_UI_WIDGETS_COLORBAR_HPP

#include "./value_map.hpp"
#include "./widget_group.hpp"
#include "./label.hpp"
#include "ui/layouts/table.hpp"
#include "ui/value_maps/affine_value_map.hpp"

namespace terraformer::ui::widgets
{
	class colorbar:private widget_group<layouts::table>
	{
	public:
		using widget_group::handle_event;
		using widget_group::get_children;
		using widget_group::get_layout;
		using widget_group::compute_size;

		template<class ValueMap, class ColorMap>
		explicit colorbar(iterator_invalidation_handler_ref iihr, ValueMap&& vm, ColorMap&& cm):
			widget_group{
				iihr,
				1u,
				layouts::table::cell_order::row_major
			},
			m_value_map{std::forward<ValueMap>(vm)},
			m_color_map{std::forward<ColorMap>(cm)}
		{
			init();
		}

		main::widget_layer_stack prepare_for_presentation(terraformer::ui::main::graphics_backend_ref);

		void theme_updated(main::config const& cfg, main::widget_instance_info)
		{
			m_null_texture = cfg.misc_textures.null;
			m_fg_tint = cfg.output_area.colors.foreground;
			m_marker_length = 8.0f * cfg.output_area.border_thickness;
			layout.params().margin_y = 0.0f;
			layout.params().margin_x = m_marker_length;
			m_border_thickness = static_cast<uint32_t>(cfg.output_area.border_thickness);
			update_frame();
		}

		void update_colorbar();

		void update_frame();

		void update_labels();

		box_size confirm_size(box_size size_in);

	private:
		type_erased_value_map m_value_map{
			std::in_place_type_t<value_maps::affine_value_map>{}, 0.0f, 1.0f
		};
		move_only_function<rgba_pixel(float)> m_color_map{
			[](float val){
				return rgba_pixel{val, val, val, 1.0f};
			}
		};

		void init();

		box_size m_size;
		main::unique_texture m_image;
		main::unique_texture m_frame;
		rgba_pixel m_fg_tint;
		float m_marker_length;
		uint32_t m_border_thickness;
		main::immutable_shared_texture m_null_texture;
		std::array<label, 13> m_labels;
	};

	class labeled_colorbar:private widget_group<layouts::table>
	{
	public:
		using widget_group::handle_event;
		using widget_group::get_children;
		using widget_group::get_layout;
		using widget_group::compute_size;
		using widget_group::confirm_size;
		using widget_group::theme_updated;
		using widget_group::prepare_for_presentation;

		template<class ValueMap, class ColorMap>
		explicit labeled_colorbar(
			iterator_invalidation_handler_ref iihr,
			std::u8string_view label,
			ValueMap&& vm, ColorMap&& cm
		):widget_group{
				iihr,
				1u,
				layouts::table::cell_order::row_major
			},
			m_colorbar{iihr, std::forward<ValueMap>(vm), std::forward<ColorMap>(cm)}
		{
			m_label.text(label);
			m_label.set_margin(0.0f);
			layout.set_record_size(1, layouts::table::cell_size::expand{});
			append(std::ref(m_label), main::widget_geometry{});
			append(std::ref(m_colorbar), main::widget_geometry{});
			is_transparent = true;
			auto const attributes = get_attributes();
			auto const widget_states = attributes.widget_states();
			widget_states[attributes.element_indices().back()].maximized = true;

		}

		void theme_updated(main::config const& new_theme, main::widget_instance_info instance_info)
		{
			widget_group::theme_updated(new_theme, instance_info);
			layout.params().margin_x = 0.0f;
			layout.params().margin_y = 0.0f;
			layout.params().no_outer_margin = false;
		}

	private:
		label m_label;
		colorbar m_colorbar;

	};

}

#endif
