//@	{"target":{"name":"widget_collection_ref.o"}}

#include "./widget_collection_ref.hpp"
#include <vector>

terraformer::ui::main::find_recursive_result
terraformer::ui::main::find_recursive(cursor_position pos, widget_collection_ref const& widgets)
{
	std::vector<widget_collection_ref> contexts;
	contexts.reserve(16);
	contexts.push_back(widgets);

	find_recursive_result retval{widget_collection_ref{}, widget_collection_view::npos};

	while(!contexts.empty())
	{
		auto const current_context = contexts.back();
		contexts.pop_back();

		auto const i = find(pos, current_context.as_view());
		if(i == widget_collection_view::npos)
		{ continue; }

		retval.widget_collection = current_context;
		retval.index = i;

		auto const widget_pointers = current_context.widget_pointers();
		auto const get_children_callbacks = current_context.get_children_callbacks();
		contexts.push_back(get_children_callbacks[i](widget_pointers[i]));
	}

	return retval;
}
