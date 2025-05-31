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
		);

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"NE",
				.value_reference = std::ref(field_value.ne)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		);

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"E",
				.value_reference = std::ref(field_value.e)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		);

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"SE",
				.value_reference = std::ref(field_value.se)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		);

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"S",
				.value_reference = std::ref(field_value.s)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		);

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"SW",
				.value_reference = std::ref(field_value.sw)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		);

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"W",
				.value_reference = std::ref(field_value.w)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		);

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"NW",
				.value_reference = std::ref(field_value.nw)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		);

		ret.create_widget(
			global_elevation_form_field{
				.label = u8"C",
				.value_reference = std::ref(field_value.c)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		);

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
		plain_midpoint_descriptor& field_value,
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
		);

		ret.create_widget(
			plain_midpoint_form_field{
				.label = u8"E",
				.value_reference = std::ref(field_value.e)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{0.0f, 1.0f}
			}
		);

		ret.create_widget(
			plain_midpoint_form_field{
				.label = u8"S",
				.value_reference = std::ref(field_value.s)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{0.0f, 1.0f}
			}
		);


		ret.create_widget(
			plain_midpoint_form_field{
				.label = u8"W",
				.value_reference = std::ref(field_value.w)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{0.0f, 1.0f}
			}
		);

		ret.create_widget(
			plain_midpoint_form_field{
				.label = u8"C",
				.value_reference = std::ref(field_value.c)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{0.0f, 1.0f}
			}
		);

		return ret;
	}

	struct plain_new_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	auto& bind(std::u8string_view field_name, plain_descriptor_new& field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			plain_new_form_field{
				.label = field_name
			}
		);

		bind(u8"Elevations/m", field_value.elevations, ret);
		bind(u8"Midpoints", field_value.midpoints, ret);

		return ret;
	}

#if 0
	struct plain_corner_descriptor_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
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
		ret.create_widget(
			global_elevation_form_field{
				.label = u8"Midpoint elevation/m",
				.value_reference = std::ref(field_value.midpoint)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		);

		return ret;
	}
#endif
}

#endif