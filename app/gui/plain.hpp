#ifndef TERRAFORMER_GUI_PLAIN_HPP
#define TERRAFORMER_GUI_PLAIN_HPP

#include "lib/generators/plain_generator/plain.hpp"
#include "ui/main/widget.hpp"
#include "ui/widgets/form.hpp"
#include "ui/widgets/table.hpp"
#include "ui/value_maps/sqrt_value_map.hpp"
#include "ui/value_maps/atan_value_map.hpp"
#include "ui/widgets/knob.hpp"
#include "ui/widgets/float_input.hpp"

namespace terraformer::app
{
	struct plain_control_point_elevation_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<float> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
	};

	struct plain_control_point_slope_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<float> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
	};

	struct plain_boundary_point_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	struct plain_elevation_table_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::table;
	};

	void bind(plain_control_point_descriptor& field_value, auto& parent)
	{
		parent.create_widget(
			plain_control_point_elevation_form_field{
				.label = u8"Elevation/m",
				.value_reference = std::ref(field_value.elevation)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::sqrt_value_map{6400.0f}
			}
		)
		.set_textbox_placeholder_string(u8"-9999.9999");
		parent.create_widget(
			plain_control_point_slope_form_field{
				.label = u8"∂/∂x",
				.value_reference = std::ref(field_value.ddx)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::atan_value_map{1.0f, true}
			}
		)
		.set_textbox_placeholder_string(u8"-0.073242545")
		.input_widget()
			.visual_angle_range(
				closed_closed_interval<geosimd::turn_angle>{
					geosimd::turns{0.5 - 0.125},
					geosimd::turns{0.5 + 0.125}
				}
			);

		parent.create_widget(
			plain_control_point_slope_form_field{
				.label = u8"∂/∂y",
				.value_reference = std::ref(field_value.ddy)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::atan_value_map{1.0f, true}
			}
		)
		.set_textbox_placeholder_string(u8"-0.073242545")
		.input_widget()
			.visual_angle_range(
				closed_closed_interval<geosimd::turn_angle>{
					geosimd::turns{0.5 - 0.125},
					geosimd::turns{0.5 + 0.125}
				}
			);
	}

	void bind(plain_control_points_info& field_value, auto& parent)
	{
		auto& n = parent.create_widget(
			plain_boundary_point_form_field{
				.label = u8"N"
			}
		);
		bind(field_value.n, n);
		n.append_pending_widgets();

		auto& ne = parent.create_widget(
			plain_boundary_point_form_field{
				.label = u8"NE"
			}
		);
		bind(field_value.ne, ne);
		ne.append_pending_widgets();

		auto& e = parent.create_widget(
			plain_boundary_point_form_field{
				.label = u8"E"
			}
		);
		bind(field_value.e, e);
		e.append_pending_widgets();

		auto& se = parent.create_widget(
			plain_boundary_point_form_field{
				.label = u8"SE"
			}
		);
		bind(field_value.se, se);
		se.append_pending_widgets();

		auto& s = parent.create_widget(
			plain_boundary_point_form_field{
				.label = u8"S"
			}
		);
		bind(field_value.s, s);
		s.append_pending_widgets();

		auto& sw = parent.create_widget(
			plain_boundary_point_form_field{
				.label = u8"SW"
			}
		);
		bind(field_value.sw, sw);
		sw.append_pending_widgets();

		auto& w = parent.create_widget(
			plain_boundary_point_form_field{
				.label = u8"W"
			}
		);
		bind(field_value.w, w);
		w.append_pending_widgets();

		auto& nw = parent.create_widget(
			plain_boundary_point_form_field{
				.label = u8"NW"
			}
		);
		bind(field_value.nw, nw);
		nw.append_pending_widgets();

		auto& c = parent.create_widget(
			plain_boundary_point_form_field{
				.label = u8"C"
			}
		);
		bind(field_value.c, c);
		c.append_pending_widgets();
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

	void bind(plain_midpoints_info& field_value, ui::widgets::form& parent)
	{
		parent.create_widget(
			plain_midpoint_form_field{
				.label = u8"N",
				.value_reference = std::ref(field_value.n)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{0.0f, 1.0f}
			}
		)
		.set_textbox_placeholder_string(u8"0.00019329926");

		parent.create_widget(
			plain_midpoint_form_field{
				.label = u8"E",
				.value_reference = std::ref(field_value.e)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{0.0f, 1.0f}
			}
		)
		.set_textbox_placeholder_string(u8"0.00019329926");

		parent.create_widget(
			plain_midpoint_form_field{
				.label = u8"S",
				.value_reference = std::ref(field_value.s)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{0.0f, 1.0f}
			}
		)
		.set_textbox_placeholder_string(u8"0.00019329926");

		parent.create_widget(
			plain_midpoint_form_field{
				.label = u8"W",
				.value_reference = std::ref(field_value.w)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{0.0f, 1.0f}
			}
		)
		.set_textbox_placeholder_string(u8"0.00019329926");

		parent.create_widget(
			plain_midpoint_form_field{
				.label = u8"C x",
				.value_reference = std::ref(field_value.c_x)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{0.0f, 1.0f}
			}
		)
		.set_textbox_placeholder_string(u8"0.00019329926");

		parent.create_widget(
			plain_midpoint_form_field{
				.label = u8"C y",
				.value_reference = std::ref(field_value.c_y)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{0.0f, 1.0f}
			}
		)
		.set_textbox_placeholder_string(u8"0.00019329926");
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

	void bind(plain_descriptor& field_value, ui::widgets::form& parent)
	{
		auto& boundary = parent.create_widget(
			plain_elevation_table_form_field{
				.label = u8"Control points"
			},
			ui::main::widget_orientation::vertical,
			std::array{
				u8"Elevation/m",
				u8"∂/∂x",
				u8"∂/∂y"
			}
		);
		bind(field_value.boundary, boundary);

		auto& midpoints = parent.create_widget(
			plain_midpoints_form_field{
				.label = u8"Midpoints"
			},
			ui::main::widget_orientation::vertical
		);
		bind(field_value.edge_midpoints, midpoints);

		parent.create_widget(
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
	}
}

#endif
