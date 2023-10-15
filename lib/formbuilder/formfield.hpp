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


	template<class BindingType, class Converter>
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

	template<class BindingType, class Converter, numeric_input_mapping_type Mapping = numeric_input_mapping_type::lin>
	struct numeric_input
	{
		static_assert(!BindingType::type::accepts_value(0.0f)
			|| Mapping != numeric_input_mapping_type::log);

		BindingType binding;
		Converter value_converter;
	};

	template<class BindingType, class Generator>
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

	template<class PixelSizeType, class HeightmapType>
	struct topographic_map_view
	{
		PixelSizeType pixel_size;
		HeightmapType heightmap;
	};
}

#endif
