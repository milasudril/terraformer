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

void terraformer::ui::main::theme_updated(
	widget_collection_view const& widgets,
	config const& cfg,
	widget_instance_info instance_info
)
{
	struct context
	{
		widget_collection_view widgets;
		widget_instance_info instance_info;
	};

	single_array<context> contexts;
	contexts.reserve(decltype(contexts)::size_type{16});
	contexts.push_back(
		context{
			.widgets = widgets,
			.instance_info = instance_info
		}
	);

	while(!contexts.empty())
	{
		auto const current_context = contexts.back();
		contexts.pop_back();

		auto const theme_updated_callbacks = current_context.widgets.theme_updated_callbacks();
		auto const widget_pointers = current_context.widgets.widget_pointers();
		auto const get_children_callbacks = current_context.widgets.get_children_const_callbacks();

		for(auto k : current_context.widgets.element_indices())
		{
			theme_updated_callbacks[k](
				widget_pointers[k],
				cfg,
				widget_instance_info{
					.section_level = current_context.instance_info.section_level,
					.paragraph_index = k.get()
				}
			);

			auto const children = get_children_callbacks[k](widget_pointers[k]);
			contexts.push_back(
				context{
					.widgets = children,
					.instance_info{
						.section_level = instance_info.section_level + 1,
						.paragraph_index = 0
					}
				}
			);
		}
	}
}