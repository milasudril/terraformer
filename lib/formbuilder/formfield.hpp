#ifndef TERRAFORMER_FORMFIELD_HPP
#define TERRAFORMER_FORMFIELD_HPP

#include <optional>

namespace terraformer
{
	template<class Widget>
	struct field
	{
		char const* name;
		char const* display_name;
		char const* description;
		Widget widget;
	};


	template<class Converter, class BindingType>
	struct textbox
	{
		Converter value_converter;
		BindingType binding;
		std::optional<int> min_width = std::nullopt;
	};

	template<class Generator, class BindingType>
	struct input_button
	{
		Generator value_generator;
		BindingType binding;
		char const* label;
		char const* description;
	};

	template<class BindingType>
	struct subform
	{
		BindingType binding;
	};

}

#endif
