#ifndef TERRAFORMER_GUI_ROLLING_HILLS_HPP
#define TERRAFORMER_GUI_ROLLING_HILLS_HPP

#include "lib/generators/rolling_hills_generator/rolling_hills_generator.hpp"
#include "lib/value_maps/log_value_map.hpp"
#include "lib/value_maps/affine_value_map.hpp"

#include "ui/widgets/form.hpp"
#include "ui/widgets/knob.hpp"
#include "ui/widgets/float_input.hpp"
#include "ui/widgets/rng_seed_input.hpp"
#include "ui/widgets/interval_input.hpp"

namespace terraformer::app
{
	struct rolling_hills_descriptor_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	struct rolling_hills_seed_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<std::array<std::byte, 16>> value_reference;
		using input_widget_type = ui::widgets::rng_seed_input<16>;
	};

	struct rolling_hills_wavelength_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<float> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
	};

	struct rolling_hills_rolloff_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<float> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
	};

	struct rolling_hills_orientation_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<float> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
	};

	struct rolling_hills_rel_z_offset_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<float> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
	};

	struct rolling_hills_amplitude_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<float> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
	};

	struct rolling_hills_shape_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
		bool expand_layout_cell;
	};

	struct rolling_hills_shape_exponent_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<float> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
	};

	struct rolling_hills_shape_clamp_to_input_range_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<closed_closed_interval<float>> value_reference;
		using input_widget_type = ui::widgets::interval_input;
		bool expand_layout_cell;
	};

	struct rolling_hills_shape_clamp_to_hardness_field
	{
		std::u8string_view label;
		std::reference_wrapper<bounded_value<open_open_interval{0.0f, 1.0f}, 1.0f - 1.0f/128.0f>> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;

		static constexpr bool is_value_valid(float value)
		{ return within(open_open_interval{0.0f, 1.0e+0f}, value); }
	};

	struct rolling_hills_shape_input_mapping_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<closed_closed_interval<float>> value_reference;
		bool expand_layout_cell;
		using input_widget_type = ui::widgets::interval_input;
	};

	struct rolling_hills_shape_clamp_to_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
		bool expand_layout_cell;
	};

	void bind(rolling_hills_filter_descriptor& field_value, ui::widgets::form& parent)
	{
		parent.create_widget(
			rolling_hills_wavelength_form_field{
				.label = u8"Wavelength x/m",
				.value_reference = std::ref(field_value.wavelength_x)
			},
			terraformer::ui::widgets::knob{
				terraformer::value_maps::log_value_map{1024.0f, 32768.0f, 2.0f}
			}
		)
		.set_textbox_placeholder_string(u8"9999.9999");

		parent.create_widget(
			rolling_hills_wavelength_form_field{
				.label = u8"Wavelength y/m",
				.value_reference = std::ref(field_value.wavelength_y)
			},
			terraformer::ui::widgets::knob{
				terraformer::value_maps::log_value_map{1024.0f, 32768.0f, 2.0f}
			}
		)
		.set_textbox_placeholder_string(u8"9999.9999");

		parent.create_widget(
			rolling_hills_rolloff_form_field{
				.label = u8"LF roll-off",
				.value_reference = std::ref(field_value.lf_rolloff)
			},
			terraformer::ui::widgets::knob{
				terraformer::value_maps::log_value_map{1.0f, 8.0f, 2.0f}
			}
		)
		.set_textbox_placeholder_string(u8"9999.9999");

		parent.create_widget(
			rolling_hills_rolloff_form_field{
				.label = u8"HF roll-off",
				.value_reference = std::ref(field_value.hf_rolloff)
			},
			terraformer::ui::widgets::knob{
				terraformer::value_maps::log_value_map{2.0f, 8.0f, 2.0f}
			}
		)
		.set_textbox_placeholder_string(u8"9999.9999");

		parent.create_widget(
			rolling_hills_orientation_form_field{
				.label = u8"Y direction",
				.value_reference = std::ref(field_value.y_direction)
			},
			terraformer::ui::widgets::knob{
				terraformer::value_maps::affine_value_map{-0.25f, 0.25f}
			}
		)
		.set_textbox_placeholder_string(u8"-0.123456789")
		.input_widget().visual_angle_range(
			closed_closed_interval<geosimd::turn_angle>{
				geosimd::turns{1.0/4.0},
				geosimd::turns{3.0/4.0}
			}
		);
	}

	void bind(rolling_hills_clamp_to_descriptor& field_value, ui::widgets::form& parent)
	{
		parent.create_widget(
			rolling_hills_shape_clamp_to_input_range_form_field{
				.label = u8"Range",
				.value_reference = std::reference_wrapper(field_value.range),
				.expand_layout_cell = true
			},
			terraformer::value_maps::affine_value_map{-1.0f, 1.0f}
		)
		.set_textbox_placeholder_string(u8"-0.123456789");

		parent.create_widget(
			rolling_hills_shape_clamp_to_hardness_field{
				.label = u8"Hardness",
				.value_reference = std::reference_wrapper(field_value.hardness)
			},
			terraformer::ui::widgets::knob{
				terraformer::value_maps::affine_value_map{0.0f, 1.0f}
			}
		);
	}

	void bind(rolling_hills_shape_descriptor& field_value, ui::widgets::form& parent)
	{
		parent.create_widget(
			rolling_hills_shape_input_mapping_form_field{
				.label = u8"Input mapping",
				.value_reference = std::reference_wrapper(field_value.input_mapping),
				.expand_layout_cell = true
			},
			terraformer::value_maps::affine_value_map{-1.0f, 1.0f}
		)
		.set_textbox_placeholder_string(u8"-0.123456789");;

		parent.create_widget(
			rolling_hills_shape_exponent_form_field{
				.label = u8"Exponent",
				.value_reference = std::reference_wrapper(field_value.exponent)
			},
			terraformer::ui::widgets::knob{
				terraformer::value_maps::log_value_map{0.25f, 4.0f, 2.0f}
			}
		)
		.set_textbox_placeholder_string(u8"0.123456789");
	}

	void bind(rolling_hills_descriptor& field_value, ui::widgets::form& parent)
	{
		parent.create_widget(
			rolling_hills_seed_form_field{
				.label = u8"Seed",
				.value_reference = std::ref(field_value.rng_seed)
			}
		);

		bind(field_value.filter, parent);

		auto& clamp_to = parent.create_widget(
			rolling_hills_shape_clamp_to_form_field{
				.label = u8"Clamp to",
				.expand_layout_cell = false
			},
			ui::main::widget_orientation::vertical
		);
		bind(field_value.clamp_to, clamp_to);

		auto& shape = parent.create_widget(
			rolling_hills_shape_form_field{
				.label = u8"Shape",
				.expand_layout_cell = false
			},
			ui::main::widget_orientation::vertical
		);
		bind(field_value.shape, shape);

		parent.create_widget(
			rolling_hills_amplitude_form_field{
				.label = u8"Amplitude/m",
				.value_reference = std::ref(field_value.amplitude)
			},
			terraformer::ui::widgets::knob{
				terraformer::value_maps::log_value_map{1.0f, 8192.0f, 2.0f}
			}
		)
		.set_textbox_placeholder_string(u8"9999.9999");

		parent.create_widget(
			rolling_hills_rel_z_offset_form_field{
				.label = u8"Relative z offset",
				.value_reference = std::ref(field_value.relative_z_offset)
			},
			terraformer::ui::widgets::knob{
				terraformer::value_maps::affine_value_map{-1.0f, 1.0f}
			}
		)
		.set_textbox_placeholder_string(u8"-0.123456789");
	}
}

#endif