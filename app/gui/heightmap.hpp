#ifndef TERRAFORMER_GUI_HEIGHTMAP_HPP
#define TERRAFORMER_GUI_HEIGHTMAP_HPP

#include "./elevation_color_map.hpp"

#include "ui/widgets/form.hpp"
#include "ui/widgets/widget_canvas.hpp"
#include "ui/widgets/false_color_image_view.hpp"
#include "ui/widgets/contour_plot.hpp"

#include "lib/pixel_store/image.hpp"

namespace terraformer::app
{
	struct heightmap_form_field
	{
		std::u8string_view label;
		bool expand_widget;
		using input_widget_type = ui::widgets::widget_canvas;
	};

	template<class View>
	struct heightmap_part_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<grayscale_image const> value_reference;
		bool expand_widget;

		using input_widget_type = View;
	};

	auto& bind(std::u8string_view field_name, std::reference_wrapper<grayscale_image const> field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			heightmap_form_field{
				.label = field_name,
				.expand_widget = true
			},
			ui::layouts::none::cell_size_mode::expand
		);

		auto& imgview = ret.create_widget(
			heightmap_part_form_field<terraformer::ui::widgets::false_color_image_view>{
				.label = u8"Heatmap",
				.value_reference = field_value,
				.expand_widget = true
			},
			terraformer::global_elevation_map,
			terraformer::get_elevation_color_lut()
		);

		auto& contours = ret.create_widget(
			heightmap_part_form_field<terraformer::ui::widgets::contour_plot>{
				.label = u8"Level curves",
				.value_reference = field_value,
				.expand_widget = true
			},
			100.0f
		);

		ret.set_refresh_function([field_value, &imgview, &contours](){
			imgview.show_image(field_value.get().pixels());
			contours.show_image(field_value.get().pixels());
		});

		return ret;
	}
}

#endif