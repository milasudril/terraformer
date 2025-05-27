#ifndef TERRAFORMER_GUI_MAP_SHEET_HPP
#define TERRAFORMER_GUI_MAP_SHEET_HPP

#include "./domain_size.hpp"
#include "lib/generators/map_sheet/map_sheet_descriptor.hpp"
#include "ui/widgets/form.hpp"

namespace terraformer::app
{
	struct map_sheet_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	auto& bind(std::u8string_view field_name, map_sheet_descriptor& field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			domain_size_form_field{
				.label = field_name,
			}
		);

		bind(u8"Domain size", field_value.domain_size, ret);

		return ret;
	}
}

#endif
