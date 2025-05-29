#ifndef TERRAFORMER_GUI_PLAIN_HPP
#define TERRAFORMER_GUI_PLAIN_HPP

#include "lib/generators/plain_generator/plain.hpp"
#include "ui/widgets/form.hpp"
#include "ui/value_maps/sqrt_value_map.hpp"
#include "ui/widgets/knob.hpp"
#include "ui/widgets/float_input.hpp"

namespace terraformer::app
{
	struct plain_corner_descriptor_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	struct global_elevation_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<float> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
	};

	struct plain_derivative_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<float> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
	};

	auto& bind(std::u8string_view field_name, plain_corner_descriptor& field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			plain_corner_descriptor_form_field{
				.label = field_name,
			},
			ui::main::widget_orientation::vertical
		);

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"Elevation/m",
				.value_reference = std::ref(field_value.elevation)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		);

		ret.create_widget(
			plain_derivative_form_field{
				.label = u8"d/dx",
				.value_reference = std::ref(field_value.ddx)
			},
			terraformer::ui::widgets::knob{terraformer::ui::value_maps::sqrt_value_map{1.0f}}
		);

		ret.create_widget(
			plain_derivative_form_field{
				.label = u8"d/dy",
				.value_reference = std::ref(field_value.ddy)
			},
			terraformer::ui::widgets::knob{terraformer::ui::value_maps::sqrt_value_map{1.0f}}
		);

		return ret;
	}

	struct plain_descriptor_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	auto& bind(std::u8string_view field_name, plain_descriptor& field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			plain_descriptor_form_field{
				.label = field_name
			}
		);

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"Elevation N/m",
				.value_reference = std::ref(field_value.n)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		);
		bind(u8"NE", field_value.ne, ret);
		ret.create_widget(
			global_elevation_form_field{
				.label = u8"Elevation E/m",
				.value_reference = std::ref(field_value.e)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		);
		bind(u8"SE", field_value.se, ret);
		ret.create_widget(
			global_elevation_form_field{
				.label = u8"Elevation S/m",
				.value_reference = std::ref(field_value.s)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		);
		bind(u8"SW", field_value.sw, ret);
		ret.create_widget(
			global_elevation_form_field{
				.label = u8"Elevation W/m",
				.value_reference = std::ref(field_value.w)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		);
		bind(u8"NW", field_value.nw, ret);

		return ret;
	}
}

#endif