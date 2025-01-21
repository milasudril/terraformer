//@	{"target":{"name":"widget_collection_ref.o"}}

#include "./widget_collection_ref.hpp"

#include "lib/array_classes/single_array.hpp"

terraformer::ui::main::find_recursive_result terraformer::ui::main::find_recursive(
	cursor_position pos,
	widget_collection_ref const& widgets,
	displacement offset
)
{
	struct context
	{
		widget_collection_ref widgets;
		displacement offset;
	};

	single_array<context> contexts;
	contexts.reserve(decltype(contexts)::size_type{16});
	contexts.push_back(
		context{
			.widgets = widgets,
			.offset = offset
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

		auto const i = find(pos, current_context.widgets.as_view(), current_context.offset);
		if(i == widget_collection_view::npos)
		{ continue; }

		retval = find_recursive_result{current_context.widgets, i};

		auto const widget_pointers = current_context.widgets.widget_pointers();
		auto const get_children_callbacks = current_context.widgets.get_children_callbacks();
		auto const widget_geometries = current_context.widgets.widget_geometries();
		contexts.push_back(
			context{
				.widgets = get_children_callbacks[i](widget_pointers[i]),
				.offset = widget_geometries[i].where + current_context.offset - location{0.0f, 0.0f, 0.0f}
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
			if(!children.element_indices().empty())
			{
				contexts.push_back(
					context{
						.widgets = children,
						.instance_info{
							.section_level = current_context.instance_info.section_level + 1,
							.paragraph_index = 0
						}
					}
				);
			}
		}
	}
}