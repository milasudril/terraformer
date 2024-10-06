//@	{"target":{"name":"widget_collection.o"}}

#include "./widget_collection.hpp"

#include "lib/array_classes/single_array.hpp"

void terraformer::ui::main::flatten(
	widget_collection_ref::reference const& element,
	widget_collection& ret
)
{
	single_array<widget_collection_ref::reference> contexts;
	contexts.push_back(element);
	while(!contexts.empty())
	{
		auto const current = contexts.back();
		contexts.pop_back();
		ret.append(current);

		auto const ptr = get_value_from_type<void*&>(current);
		auto const get_children = get_value_from_type<get_children_callback&>(current);
		auto const children = get_children(ptr);
		for(auto k = children.first_element_index(); k != std::size(children); ++k)
		{
			auto const index = children.first_element_index() + (children.last_element_index() - k);
			contexts.push_back(children.extract(index));
		}
	}
}


terraformer::ui::main::widget_collection terraformer::ui::main::flatten(widget_collection_ref const& root)
{
	widget_collection ret;
	for(auto k = root.first_element_index(); k != std::size(root); ++k)
	{ flatten(root.extract(k), ret); }
	return ret;
}
