#ifndef TERRAFORMER_GUI_DOMAIN_SIZE_HPP
#define TERRAFORMER_GUI_DOMAIN_SIZE_HPP

#include "ui/widgets/form.hpp"
#include "ui/widgets/knob.hpp"
#include "ui/widgets/float_input.hpp"
#include "ui/value_maps/log_value_map.hpp"

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

	auto& bind(std::u8string_view field_name, domain_size_descriptor& field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			domain_size_form_field{
				.label = field_name,
			}
		);

		ret.create_widget(
			domain_length_form_field{
				.label = u8"Width/m",
				.value_reference = std::ref(field_value.width)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::log_value_map{1.0f, 65536.0f, 2.0f}
			}
		);

		ret.create_widget(
			domain_length_form_field{
				.label = u8"Height/m",
				.value_reference = std::ref(field_value.height)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::log_value_map{1.0f, 65536.0f, 2.0f}
			}
		);

		return ret;
	}
}

#endif
