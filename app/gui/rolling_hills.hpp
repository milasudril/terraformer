#ifndef TERRAFORMER_GUI_ROLLING_HILLS_HPP
#define TERRAFORMER_GUI_ROLLING_HILLS_HPP

#include "lib/generators/rolling_hills_generator/rolling_hills_generator.hpp"
#include "ui/widgets/form.hpp"
#include "ui/value_maps/log_value_map.hpp"
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
	};

	struct rolling_hills_shape_exponent_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<float> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
	};

	struct rolling_hills_shape_clamp_to_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<closed_closed_interval<float>> value_reference;
		using input_widget_type = ui::widgets::interval_input;
	};

	struct rolling_hills_shape_input_mapping_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<closed_closed_interval<float>> value_reference;
		using input_widget_type = ui::widgets::interval_input;
	};

	auto& bind(std::u8string_view field_name, rolling_hills_shape_descriptor& field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			rolling_hills_shape_form_field{
				.label = field_name
			},
			ui::main::widget_orientation::vertical
		);

		ret.create_widget(
			rolling_hills_shape_clamp_to_form_field{
				.label = u8"Clamp to",
				.value_reference = std::reference_wrapper(field_value.clamp_to)
			}
		);

		ret.create_widget(
			rolling_hills_shape_input_mapping_form_field{
				.label = u8"Input mapping",
				.value_reference = std::reference_wrapper(field_value.input_mapping)
			}
		);

		ret.create_widget(
			rolling_hills_shape_exponent_form_field{
				.label = u8"Exponent",
				.value_reference = std::reference_wrapper(field_value.exponent)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::log_value_map{0.25f, 4.0f, 2.0f}
			}
		);

		return ret;
	}

	auto& bind(std::u8string_view field_name, rolling_hills_descriptor& field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			rolling_hills_descriptor_form_field{
				.label = field_name,
			}
		);

		ret.create_widget(
			rolling_hills_seed_form_field{
				.label = u8"Seed",
				.value_reference = std::ref(field_value.rng_seed)
			}
		);

		ret.create_widget(
			rolling_hills_wavelength_form_field{
				.label = u8"Wavelength x/m",
				.value_reference = std::ref(field_value.filter.wavelength_x)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::log_value_map{1024.0f, 32768.0f, 2.0f}
			}
		);

		ret.create_widget(
			rolling_hills_wavelength_form_field{
				.label = u8"Wavelength y/m",
				.value_reference = std::ref(field_value.filter.wavelength_y)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::log_value_map{1024.0f, 32768.0f, 2.0f}
			}
		);

		ret.create_widget(
			rolling_hills_rolloff_form_field{
				.label = u8"LF roll-off",
				.value_reference = std::ref(field_value.filter.lf_rolloff)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{1.0f, 4.0f}
			}
		);

		ret.create_widget(
			rolling_hills_rolloff_form_field{
				.label = u8"HF roll-off",
				.value_reference = std::ref(field_value.filter.hf_rolloff)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{2.0f, 4.0f}
			}
		);

		ret.create_widget(
			rolling_hills_orientation_form_field{
				.label = u8"Y direction",
				.value_reference = std::ref(field_value.filter.y_direction)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{-0.25f, 0.25f}
			}
		).input_widget().visual_angle_range(
			closed_closed_interval<geosimd::turn_angle>{
				geosimd::turns{1.0/4.0},
				geosimd::turns{3.0/4.0}
			}
		);

		bind(u8"Shape", field_value.shape, ret);

		ret.create_widget(
			rolling_hills_amplitude_form_field{
				.label = u8"Amplitude/m",
				.value_reference = std::ref(field_value.amplitude)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::log_value_map{1.0f, 8192.0f, 2.0f}
			}
		);

		ret.create_widget(
			rolling_hills_rel_z_offset_form_field{
				.label = u8"Relative z offset",
				.value_reference = std::ref(field_value.relative_z_offset)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::affine_value_map{-1.0f, 1.0f}
			}
		);


		return ret;
	}
}

#endif