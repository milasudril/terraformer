#ifndef TERRAFORMER_GUI_DOMAIN_SIZE_HPP
#define TERRAFORMER_GUI_DOMAIN_SIZE_HPP

#include "ui/widgets/form.hpp"
#include "ui/widgets/knob.hpp"
#include "ui/widgets/float_input.hpp"

#include "lib/value_maps/log_value_map.hpp"
#include "lib/generators/domain/domain_size.hpp"

namespace terraformer::app
{
	struct domain_length_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<float> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
	};

	struct domain_size_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	void bind(domain_size_descriptor& field_value, ui::widgets::form& parent)
	{
		parent.create_widget(
			domain_length_form_field{
				.label = u8"Width/m",
				.value_reference = std::ref(field_value.width)
			},
			terraformer::ui::widgets::knob{
				terraformer::value_maps::log_value_map{1.0f, 65536.0f, 2.0f}
			}
		)
		.set_textbox_placeholder_string(u8"9999.9999");

		parent.create_widget(
			domain_length_form_field{
				.label = u8"Height/m",
				.value_reference = std::ref(field_value.height)
			},
			terraformer::ui::widgets::knob{
				terraformer::value_maps::log_value_map{1.0f, 65536.0f, 2.0f}
			}
		)
		.set_textbox_placeholder_string(u8"9999.9999");
	}
}

#endif
