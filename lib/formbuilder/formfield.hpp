#ifndef TERRAFORMER_FORMFIELD_HPP
#define TERRAFORMER_FORMFIELD_HPP

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
	};

	template<class Callable, class BindingType>
	struct text_display
	{
		Callable source;
		BindingType binding;
	};

	template<class BindingType>
	struct subform
	{
		BindingType binding;
	};

	template<class Callable, class BindingType>
	struct button
	{
		Callable on_activated;
		BindingType binding;  // To be "read" by on_activated
		char const* label;
		char const* description;
	};

	template<class TextWidget, class CommandWidget, class BindingType>
	struct picker
	{
		TextWidget text_widget;
		CommandWidget command_widget;
		BindingType binding;
	};
}

#endif