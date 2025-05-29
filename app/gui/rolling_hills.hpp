#ifndef TERRAFORMER_GUI_ROLLING_HILLS_HPP
#define TERRAFORMER_GUI_ROLLING_HILLS_HPP

#include "lib/generators/rolling_hills_generator/rolling_hills_generator.hpp"
#include "ui/widgets/form.hpp"
#include "ui/value_maps/log_value_map.hpp"
#include "ui/widgets/knob.hpp"
#include "ui/widgets/float_input.hpp"
#include "ui/widgets/text_to_bytearray_input.hpp"

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
		using input_widget_type = ui::widgets::text_to_bytearray_input<16>;
	};

	struct rolling_hills_wavelength_form_field
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

	struct rolling_hills_shape_form_field
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
				.value_reference = std::ref(field_value.wavelength_x)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::log_value_map{1024.0f, 32768.0f, 2.0f}
			}
		);

		ret.create_widget(
			rolling_hills_wavelength_form_field{
				.label = u8"Wavelength y/m",
				.value_reference = std::ref(field_value.wavelength_y)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::log_value_map{1024.0f, 32768.0f, 2.0f}
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

		ret.create_widget(
			rolling_hills_shape_form_field{
				.label = u8"Shape",
				.value_reference = std::ref(field_value.shape)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::log_value_map{1.0f/4.0f, 4.0f, 2.0f}
			}
		);

		ret.create_widget(
			rolling_hills_amplitude_form_field{
				.label = u8"Amplitude/m",
				.value_reference = std::ref(field_value.amplitude)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::log_value_map{1.0f, 8192.0f, 2.0f}
			}
		);


		return ret;
	}
}

#endif