//@	{"target":{"name":"widget_reference.o"}}

#include "./widget_reference.hpp"

void terraformer::ui::main::flatten(widget_reference const& widget, single_array<widget_reference>& ret)
{
	single_array<widget_reference> contexts;
	contexts.push_back(widget);
	while(!contexts.empty())
	{
		auto const current = contexts.back();
		contexts.pop_back();
		ret.push_back(current);

		auto const widget_pointers = current.collection().widget_pointers();
		auto const get_children = current.collection().get_children_const_callbacks();
		auto const index = current.index();
		auto const children = get_children[index](widget_pointers[index]);
		for(auto k = children.first_element_index(); k != std::size(children); ++k)
		{
			auto const index = children.first_element_index() + (children.last_element_index() - k);
			contexts.push_back(widget_reference{children, index});
		}
	}
}


terraformer::single_array<terraformer::ui::main::widget_reference>
terraformer::ui::main::flatten(widget_collection_view const& root)
{
	terraformer::single_array<terraformer::ui::main::widget_reference> ret;
	for(auto k = root.first_element_index(); k != std::size(root); ++k)
	{ flatten(widget_reference{root, k}, ret); }
	return ret;
}
