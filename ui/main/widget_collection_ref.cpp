//@	{"target":{"name":"widget_collection_ref.o"}}

#include "./widget_collection_ref.hpp"

#include "lib/array_classes/single_array.hpp"

terraformer::ui::main::find_recursive_result
terraformer::ui::main::find_recursive(cursor_position pos, widget_collection_ref const& widgets)
{
	struct context
	{
		widget_collection_ref widgets;
	};

	single_array<context> contexts;
	contexts.reserve(decltype(contexts)::size_type{16});
	contexts.push_back(
		context{
			.widgets = widgets
		}
	);

	find_recursive_result retval{
		widget_collection_ref{},
		widget_collection_view::npos
	};

	widget_collection_ref::index_type global_index{0};

	while(!contexts.empty())
	{
		auto const current_context = contexts.back();
		contexts.pop_back();

		auto const i = find(pos, current_context.widgets.as_view());
		if(i == widget_collection_view::npos)
		{ continue; }

		retval = find_recursive_result{current_context.widgets, i};

		auto const widget_pointers = current_context.widgets.widget_pointers();
		auto const get_children_callbacks = current_context.widgets.get_children_callbacks();
		contexts.push_back(
			context{
				.widgets = get_children_callbacks[i](widget_pointers[i])
			}
		);
	}

	return retval;
}
