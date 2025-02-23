//@	{"target":{"name":"widget_collection_ref.o"}}

#include "./widget_collection_ref.hpp"

#include "lib/array_classes/single_array.hpp"

terraformer::scaling
terraformer::ui::main::run(minimize_cell_width_context const& ctxt)
{
	auto const initial_size = ctxt.compute_size(ctxt.current_widget, widget_height_request{});

	auto const widget_pointers = ctxt.children.widget_pointers();
	auto const widget_states = ctxt.children.widget_states();
	auto const get_children_callbacks = ctxt.children.get_children_callbacks();
	auto const get_layout_callbacks = ctxt.children.get_layout_callbacks();
	auto const size_callbacks = ctxt.children.compute_size_given_width_callbacks();
	auto const sizes = ctxt.children.sizes();
	for(auto k : ctxt.children.element_indices())
	{
		if(!widget_states[k].collapsed) [[likely]]
		{
			sizes[k] = run(minimize_cell_width_context{
				.current_widget = widget_pointers[k],
				.compute_size = size_callbacks[k],
				.children = get_children_callbacks[k](widget_pointers[k]),
				.current_layout = get_layout_callbacks[k](widget_pointers[k])
			});
		}
		else
		{ sizes[k] = scaling{0.0f, 0.0f, 0.0f}; }
	}

	auto const size_from_layout = ctxt.current_layout.set_default_cell_sizes(ctxt.children.as_view());

	return scaling{
		std::max(initial_size[0], size_from_layout[0]),
		std::max(initial_size[1], size_from_layout[1]),
		std::max(initial_size[2], size_from_layout[2])
	};
}

terraformer::scaling
terraformer::ui::main::run(minimize_cell_height_context const& ctxt)
{
	auto const initial_size = ctxt.compute_size(ctxt.current_widget, widget_width_request{});

	auto const widget_pointers = ctxt.children.widget_pointers();
	auto const widget_states = ctxt.children.widget_states();
	auto const get_children_callbacks = ctxt.children.get_children_callbacks();
	auto const get_layout_callbacks = ctxt.children.get_layout_callbacks();
	auto const size_callbacks = ctxt.children.compute_size_given_height_callbacks();
	auto const sizes = ctxt.children.sizes();
	for(auto k : ctxt.children.element_indices())
	{
		if(!widget_states[k].collapsed) [[likely]]
		{
			sizes[k] = run(minimize_cell_height_context{
				.current_widget = widget_pointers[k],
				.compute_size = size_callbacks[k],
				.children = get_children_callbacks[k](widget_pointers[k]),
				.current_layout = get_layout_callbacks[k](widget_pointers[k])
			});
		}
		else
		{ sizes[k] = scaling{0.0f, 0.0f, 0.0f}; }
	}

	auto const size_from_layout = ctxt.current_layout.set_default_cell_sizes(ctxt.children.as_view());

	return scaling{
		std::max(initial_size[0], size_from_layout[0]),
		std::max(initial_size[1], size_from_layout[1]),
		std::max(initial_size[2], size_from_layout[2])
	};
}

terraformer::scaling
terraformer::ui::main::run(adjust_cell_sizes_context const& ctxt, scaling available_size)
{
	auto const new_size = ctxt.current_layout.adjust_cell_sizes(available_size);
	auto const widget_pointers = ctxt.children.widget_pointers();
	auto const widget_states = ctxt.children.widget_states();
	auto const get_children_callbacks = ctxt.children.get_children_callbacks();
	auto const get_layout_callbacks = ctxt.children.get_layout_callbacks();
	for(auto k : ctxt.children.element_indices())
	{
		if(!widget_states[k].collapsed) [[likely]]
		{
			run(
				adjust_cell_sizes_context{
					.current_widget = widget_pointers[k],
					.children = get_children_callbacks[k](widget_pointers[k]),
					.current_layout = get_layout_callbacks[k](widget_pointers[k])
				},
				new_size
			);
		}
	}
	return new_size;
}

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
		widget_collection_view::npos,
		offset
	};

	while(!contexts.empty())
	{
		auto const current_context = contexts.back();
		contexts.pop_back();

		auto const i = find(pos, current_context.widgets.as_view(), current_context.offset);
		if(i == widget_collection_view::npos)
		{ continue; }

		retval = find_recursive_result{
			current_context.widgets, i, current_context.offset
		};

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
