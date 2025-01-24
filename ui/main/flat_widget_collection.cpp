//@	{"target":{"name":"flat_widget_collection.o"}}

#include "./flat_widget_collection.hpp"

void terraformer::ui::main::flatten(widget_tree_address const& widget, flat_widget_collection& ret)
{
	struct context
	{
		widget_tree_address address;
		widget_tree_address_parent parent_address;
	};
	single_array<context> contexts;
	contexts.push_back(
		context{
			.address = widget,
			.parent_address = widget_tree_address_parent{}
		}
	);
	while(!contexts.empty())
	{
		auto const current = contexts.back();
		contexts.pop_back();

		auto const index = current.address.index();
		auto const widget_pointers = current.address.collection().widget_pointers();
		auto const kbe_callbacks = current.address.collection().event_callbacks<keyboard_button_event, window_ref, ui_controller>();
		auto const kfe_callbacks = current.address.collection().event_callbacks<keyboard_focus_enter_event, window_ref, ui_controller>();
		auto const kfl_callbacks = current.address.collection().event_callbacks<keyboard_focus_leave_event, window_ref, ui_controller>();
		auto const typing_callbacks = current.address.collection().event_callbacks<typing_event, window_ref, ui_controller>();
		auto const widget_states = current.address.collection().widget_states();
		auto const ptr = widget_pointers[index];
		ret.append(
			ptr,
			current.address,
			current.parent_address,
			widget_states[index],
			kbe_callbacks[index],
			typing_callbacks[index],
			kfe_callbacks[index],
			kfl_callbacks[index]
		);

		widget_tree_address_parent const parent_address{ret.attributes(), ret.element_indices().back()};

		auto const get_children = current.address.collection().get_children_const_callbacks();
		auto const children = get_children[index](ptr);
		auto const indices = children.element_indices();
		for(auto k : indices)
		{
			auto const index_child = indices.front() + (indices.back() - k);
			contexts.push_back(
				context{
					.address = widget_tree_address{children, index_child},
					.parent_address = parent_address
				}
			);
		}
	}
}


terraformer::ui::main::flat_widget_collection
terraformer::ui::main::flatten(widget_collection_view const& root)
{
	flat_widget_collection ret;
	for(auto k : root.element_indices())
	{ flatten(widget_tree_address{root, k}, ret); }
	return ret;
}