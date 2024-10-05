//@	{"target":{"name":"widget_collection.o"}}

#include "./widget_collection.hpp"

#include "lib/array_classes/single_array.hpp"

terraformer::ui::main::widget_collection terraformer::ui::main::flatten(widget_collection_ref const& root)
{
	widget_collection ret;
	single_array<widget_collection_ref> contexts;
	contexts.push_back(root);
	while(!contexts.empty())
	{
		auto const current_context = contexts.back();
		contexts.pop_back();

		for(auto k = current_context.first_element_index(); k != std::size(current_context); ++k)
		{
			ret.append(current_context.extract(k));
			auto const widget_pointers = current_context.widget_pointers();
			auto const get_children_callbacks = current_context.get_children_callbacks();
			contexts.push_back(get_children_callbacks[k](widget_pointers[k]));
		}
	}

	return ret;
}
