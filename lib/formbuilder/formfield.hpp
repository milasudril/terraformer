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
		BindingType binding;
		Converter value_converter;
		std::optional<int> min_width = std::nullopt;
	};

	enum class numeric_input_mapping_type{lin, log};

	template<class BindingType>
	requires(std::is_same_v<std::remove_cvref_t<typename BindingType::type>, float>)
	struct knob
	{
		BindingType binding;
		float min;
		float max;
		numeric_input_mapping_type mapping = numeric_input_mapping_type::lin;
	};

	template<class Generator, class BindingType>
	struct input_button
	{
		BindingType binding;
		Generator value_generator;
		char const* label;
		char const* description;
	};

	template<class BindingType>
	struct subform
	{
		BindingType binding;
	};

	template<class HeightmapType, class PixelSizeType>
	struct topographic_map_view
	{
		PixelSizeType pixel_size;
		HeightmapType heightmap;
	};
}

#endif
