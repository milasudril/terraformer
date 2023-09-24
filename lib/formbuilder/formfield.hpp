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

	template<class BindingType>
	requires(std::is_same_v<std::remove_cvref_t<typename BindingType::type>, float>)
	struct knob
	{
		float min;
		float max;
		BindingType binding;
		enum class mapping_type{lin, log};
		mapping_type mapping = mapping_type::lin;
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

	template<class BindingType>
	struct topographic_map_view
	{
		BindingType binding;
	};
}

#endif
