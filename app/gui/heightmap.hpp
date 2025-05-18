#ifndef TERRAFORMER_GUI_HEIGHTMAP_HPP
#define TERRAFORMER_GUI_HEIGHTMAP_HPP

#include "./elevation_color_map.hpp"

#include "ui/widgets/form.hpp"
#include "ui/widgets/false_color_image_view.hpp"

#include "lib/pixel_store/image.hpp"

namespace terraformer::app
{
	struct heightmap_field_descriptor
	{
		std::u8string_view label;
		std::reference_wrapper<grayscale_image const> value_reference;
		bool expand_widget;

		using input_widget_type = terraformer::ui::widgets::false_color_image_view;
	};

	auto& bind(std::u8string_view field_name, std::reference_wrapper<grayscale_image const> field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			heightmap_field_descriptor{
				.label = field_name,
				.value_reference = field_value,
				.expand_widget = true
			},
			terraformer::global_elevation_map,
			terraformer::get_elevation_color_lut()
		);

		return ret;
	}
}

#endif