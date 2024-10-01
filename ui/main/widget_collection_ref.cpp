//@	{"target":{"name":"widget_collection_ref.o"}}

#include "./widget_collection_ref.hpp"

terraformer::ui::main::find_recursive_result
terraformer::ui::main::find_recursive(cursor_position pos, widget_collection_ref const& widgets)
{
	// Is pos within any widget at this level
	auto const i = find(pos, widgets.as_view());
	if(i == widget_collection_view::npos) [[likely]]
	{
		// No, return empty
		return find_recursive_result{widget_collection_ref{}, widget_collection_view::npos};
	}

	// Is pos within any child widget
	auto const widget_pointers = widgets.widget_pointers();
	auto const get_children_callbacks = widgets.get_children_callbacks();
	auto const j = find_recursive(pos, get_children_callbacks[i](widget_pointers[i]));
	if(j.index == widget_collection_view::npos) [[unlikely]]
	{
		// No, return the widget at current level
		return find_recursive_result{widgets, i};
	}

	// Yes, return the widget at child level
	return j;
}
