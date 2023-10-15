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

	template<class BindingType, numeric_input_mapping_type Mapping = numeric_input_mapping_type::lin>
	struct knob
	{
		BindingType binding;
		static constexpr auto mapping()
		{ return Mapping; }

		BindingType::type::value_type min;
		BindingType::type::value_type max;
	};

	template<class BindingType, class Converter, numeric_input_mapping_type Mapping = numeric_input_mapping_type::lin>
	struct numeric_input
	{
		static_assert(!BindingType::type::accepts_value(0.0f)
			|| Mapping != numeric_input_mapping_type::log);

		static constexpr auto mapping()
		{ return Mapping; }

		BindingType binding;
		Converter value_converter;
	};

	template<class BindingType, class Converter>
	using numeric_input_log = numeric_input<BindingType, Converter, numeric_input_mapping_type::log>;

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

	template<class BindingType, class Converter, class LabelContainer>
	struct enum_input
	{
		BindingType binding;
		Converter value_converter;
		LabelContainer labels;
	};
}

#endif
