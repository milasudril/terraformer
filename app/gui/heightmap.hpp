#ifndef TERRAFORMER_GUI_HEIGHTMAP_HPP
#define TERRAFORMER_GUI_HEIGHTMAP_HPP

#include "./elevation_color_map.hpp"

#include "ui/widgets/form.hpp"
#include "ui/widgets/widget_canvas.hpp"
#include "ui/widgets/heatmap_view.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/knob.hpp"
#include "ui/widgets/float_input.hpp"
#include "ui/value_maps/log_value_map.hpp"

#include "lib/pixel_store/image.hpp"

namespace terraformer::app
{
	struct level_curves_descriptor
	{
		float interval = 100.0f;
		bool visible = true;
	};

	struct level_curves_descriptor_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	struct level_curves_visible_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<bool> value_reference;
		bool label_on_input_field;
		using input_widget_type = ui::widgets::toggle_button;
	};

	struct level_curves_interval_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<float> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
	};

	auto& bind(std::u8string_view field_name, level_curves_descriptor& field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			level_curves_descriptor_form_field{
				.label = field_name
			}
		);

		ret.create_widget(
			level_curves_visible_form_field{
				.label = u8"Visible",
				.value_reference = std::ref(field_value.visible),
				.label_on_input_field = true
			}
		);

		ret.create_widget(
			level_curves_interval_form_field{
				.label = u8"Interval/m",
				.value_reference = std::ref(field_value.interval)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::log_value_map{1.0f, 1024.0f, 2.0f}
			}
		);

		return ret;
	}

	struct heightmap_descriptor
	{
		grayscale_image data;
		level_curves_descriptor level_curves;
	};

	struct heightmap_chart_form_field
	{
		std::u8string_view label;
		bool expand_layout_cell;
		using input_widget_type = ui::widgets::form;
	};

	struct heightmap_data_area_form_field
	{
		std::u8string_view label;
		bool expand_layout_cell;
		using input_widget_type = ui::widgets::widget_canvas;
	};

	template<class View>
	struct heightmap_part_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<grayscale_image const> value_reference;
		bool expand_layout_cell;
		bool maximize_widget;

		using input_widget_type = View;
	};

	struct level_curves_form_field
	{
		std::u8string_view label;
		using input_widget_type = terraformer::ui::widgets::form;
	};

	auto& bind(std::u8string_view field_name, heightmap_descriptor& field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			heightmap_chart_form_field{
				.label = field_name,
				.expand_layout_cell = true
			}
		);

		auto& imgview = ret.create_widget(
			heightmap_part_form_field<terraformer::ui::widgets::heatmap_view>{
				.label = u8"Heatmap",
				.value_reference = std::as_const(field_value.data),
				.expand_layout_cell = true,
				.maximize_widget = true
			},
			u8"Elevation/m",
			terraformer::global_elevation_map,
			terraformer::get_elevation_color_lut()
		);

		auto& level_curves = bind(u8"Level curves", field_value.level_curves, ret);
		level_curves.on_content_updated([&level_curves = field_value.level_curves, &imgview](auto&&...){
			if(level_curves.visible)
			{ imgview.show_level_curves(); }
			else
			{ imgview.hide_level_curves(); }

			imgview.set_level_curve_interval(level_curves.interval);
		});

		ret.set_refresh_function([&field_value, &imgview](){
			imgview.show_image(std::as_const(field_value).data.pixels());
		});

		return ret;
	}
}

#endif