#ifndef TERRAFORMER_GUI_PLAIN_HPP
#define TERRAFORMER_GUI_PLAIN_HPP

#include "lib/generators/plain_generator/plain.hpp"
#include "ui/widgets/form.hpp"
#include "ui/value_maps/sqrt_value_map.hpp"
#include "ui/widgets/knob.hpp"
#include "ui/widgets/float_input.hpp"

namespace terraformer::app
{
	struct global_elevation_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<float> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
	};

	struct plain_elevations_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	auto& bind(
		std::u8string_view field_name,
		plain_control_point_elevation_descriptor& field_value,
		ui::widgets::form& form
	)
	{
		auto& ret = form.create_widget(
			plain_elevations_form_field{
				.label = field_name
			},
			ui::main::widget_orientation::vertical
		);

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"N",
				.value_reference = std::ref(field_value.n)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		)
		.set_textbox_placeholder_string(u8"-9999.9999");

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"NE",
				.value_reference = std::ref(field_value.ne)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		)
		.set_textbox_placeholder_string(u8"-9999.9999");

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"E",
				.value_reference = std::ref(field_value.e)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		)
		.set_textbox_placeholder_string(u8"-9999.9999");

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"SE",
				.value_reference = std::ref(field_value.se)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		)
		.set_textbox_placeholder_string(u8"-9999.9999");

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"S",
				.value_reference = std::ref(field_value.s)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		)
		.set_textbox_placeholder_string(u8"-9999.9999");

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"SW",
				.value_reference = std::ref(field_value.sw)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		)
		.set_textbox_placeholder_string(u8"-9999.9999");

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"W",
				.value_reference = std::ref(field_value.w)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		)
		.set_textbox_placeholder_string(u8"-9999.9999");

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"NW",
				.value_reference = std::ref(field_value.nw)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		)
		.set_textbox_placeholder_string(u8"-9999.9999");

		return ret;
	}

	struct plain_midpoint_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<bounded_value<open_open_interval{0.0f, 1.0e+0f}, 5.0e-1f>> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;

		static constexpr bool is_value_valid(float value)
		{ return within(open_open_interval{0.0f, 1.0e+0f}, value); }
	};

	struct plain_midpoints_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	auto& bind(
		std::u8string_view field_name,
		plain_edge_midpoint_descriptor& field_value,
		ui::widgets::form& form
	)
	{
		auto& ret = form.create_widget(
			plain_elevations_form_field{
				.label = field_name
			},
			ui::main::widget_orientation::vertical
		);

		ret.create_widget(
			plain_midpoint_form_field{
				.label = u8"N",
				.value_reference = std::ref(field_value.n)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{0.0f, 1.0f}
			}
		)
		.set_textbox_placeholder_string(u8"0.00019329926");

		ret.create_widget(
			plain_midpoint_form_field{
				.label = u8"E",
				.value_reference = std::ref(field_value.e)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{0.0f, 1.0f}
			}
		)
		.set_textbox_placeholder_string(u8"0.00019329926");

		ret.create_widget(
			plain_midpoint_form_field{
				.label = u8"S",
				.value_reference = std::ref(field_value.s)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{0.0f, 1.0f}
			}
		)
		.set_textbox_placeholder_string(u8"0.00019329926");

		ret.create_widget(
			plain_midpoint_form_field{
				.label = u8"W",
				.value_reference = std::ref(field_value.w)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{0.0f, 1.0f}
			}
		)
		.set_textbox_placeholder_string(u8"0.00019329926");

		return ret;
	}

	struct plain_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	struct global_orientation_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<float> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
	};

	auto& bind(std::u8string_view field_name, plain_descriptor& field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			plain_form_field{
				.label = field_name
			}
		);

		bind(u8"Elevations/m", field_value.elevations, ret);
		bind(u8"Edge midpoints", field_value.edge_midpoints, ret);

		ret.create_widget(
			global_orientation_form_field{
				.label = u8"Orientation",
				.value_reference = std::ref(field_value.orientation)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{-0.5f, 0.5f}
			}
		)
		.set_textbox_placeholder_string(u8"-0.123456789")
		.input_widget().visual_angle_range(
			closed_closed_interval<geosimd::turn_angle>{
				geosimd::turns{0.0},
				geosimd::turns{1.0}
			}
		);

		return ret;
	}
}

#endif