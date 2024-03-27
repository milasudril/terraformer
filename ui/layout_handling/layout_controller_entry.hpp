#ifndef TERRAFORMER_UI_LAYOUT_HANDLING_LAYOUT_CONTROLLER_ENTRY_HPP
#define TERRAFORMER_UI_LAYOUT_HANDLING_LAYOUT_CONTROLLER_ENTRY_HPP

namespace terraformer::ui::layout_handling
{
	struct widget_dimensions
	{
		float width;
		float height;
	};

	template<class T>
	concept layout_controller_entry = requires (T const& obj, widget_dimensions dim)
	{
		{obj.get_dimensions(dim)}->std::same_as<widget_dimensions>;
	};
}

#endif