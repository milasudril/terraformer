#ifndef TERRAFORMER_GUI_HEIGHTMAP_HPP
#define TERRAFORMER_GUI_HEIGHTMAP_HPP

#include "./elevation_color_map.hpp"

#include "ui/widgets/form.hpp"
#include "ui/widgets/widget_canvas.hpp"
#include "ui/widgets/false_color_image_view.hpp"
#include "ui/widgets/contour_plot.hpp"
#include "ui/widgets/colorbar.hpp"

#include "lib/pixel_store/image.hpp"

namespace terraformer::app
{
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
		bool maximize_widget;

		using input_widget_type = View;
	};

	struct colorbar_form_field
	{
		std::u8string_view label;
		bool maximize_widget;
		using input_widget_type = terraformer::ui::widgets::colorbar;
	};

	auto& bind(std::u8string_view field_name, std::reference_wrapper<grayscale_image const> field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			heightmap_chart_form_field{
				.label = field_name,
				.expand_layout_cell = true
			},
			ui::main::widget_orientation::horizontal
		);

		auto& data_area = ret.create_widget(
			heightmap_data_area_form_field{
				.label = u8"",
				.expand_layout_cell = true
			},
			ui::layouts::none::cell_size_mode::expand
		);


		auto& imgview = data_area.create_widget(
			heightmap_part_form_field<terraformer::ui::widgets::false_color_image_view>{
				.label = u8"Heatmap",
				.value_reference = field_value,
				.maximize_widget = true
			},
			terraformer::global_elevation_map,
			terraformer::get_elevation_color_lut()
		);

		auto& contours = data_area.create_widget(
			heightmap_part_form_field<terraformer::ui::widgets::contour_plot>{
				.label = u8"Level curves",
				.value_reference = field_value,
				.maximize_widget = true
			},
			100.0f
		);

		data_area.set_refresh_function([field_value, &imgview, &contours](){
			imgview.show_image(field_value.get().pixels());
			contours.show_image(field_value.get().pixels());
		});

		ret.create_widget(
			colorbar_form_field{
				.label = u8"Elevation/m",
				.maximize_widget = true
			},
			global_elevation_map,
			get_elevation_color_lut()
		);

		return data_area;
	}
}

#endif