//@	{"target":{"name":"widget_reference.o"}}

#include "./widget_reference.hpp"

void terraformer::ui::main::flatten(widget_tree_address const& widget, flat_widget_collection_view& ret)
{
	single_array<widget_tree_address> contexts;
	contexts.push_back(widget);
	while(!contexts.empty())
	{
		auto const current = contexts.back();
		contexts.pop_back();

		auto const index = current.index();
		auto const widget_pointers = current.collection().widget_pointers();
		auto const kbe_callbacks = current.collection().event_callbacks<keyboard_button_event>();
		auto const ptr = widget_pointers[index];
		ret.append(ptr, current, kbe_callbacks[index]);

		auto const get_children = current.collection().get_children_const_callbacks();
		auto const children = get_children[index](widget_pointers[index]);
		for(auto k = children.first_element_index(); k != std::size(children); ++k)
		{
			auto const index = children.first_element_index() + (children.last_element_index() - k);
			contexts.push_back(widget_tree_address{children, index});
		}
	}
}


terraformer::ui::main::flat_widget_collection_view
terraformer::ui::main::flatten(widget_collection_view const& root)
{
	flat_widget_collection_view ret;
	for(auto k = root.first_element_index(); k != std::size(root); ++k)
	{ flatten(widget_tree_address{root, k}, ret); }
	return ret;
}
