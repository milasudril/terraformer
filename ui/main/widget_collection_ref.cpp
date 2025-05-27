//@	{"target":{"name":"widget_collection_ref.o"}}

#include "./widget_collection_ref.hpp"

#include "lib/array_classes/single_array.hpp"


terraformer::box_size terraformer::ui::main::run(minimize_cell_size_context const& ctxt)
{
	// TODO: Decide which dimension to minimize. Should be determined by parent

	auto const initial_size = ctxt.compute_size(widget_width_request{});

	auto& children = ctxt.children();
	auto const widget_states = children.widget_states();
	auto const sizes = children.sizes();
	for(auto k : children.element_indices())
	{
		if(widget_states[k].collapsed) [[unlikely]]
		{ sizes[k] = box_size{}; }
		else
		{ sizes[k] = run(minimize_cell_size_context{children, k}); }
	}

	auto const layout = ctxt.get_layout();
	if(!layout.is_valid())
	{ return initial_size; }

	layout.set_default_cell_sizes_to(sizes);
	auto const size_from_layout = layout.get_dimensions();

	return max(initial_size, size_from_layout);
}

terraformer::box_size terraformer::ui::main::run(adjust_cell_sizes_context const& ctxt, box_size available_size)
{
	auto const layout = ctxt.get_layout();
	auto const children = ctxt.children();
	auto const widget_states = children.widget_states();
	auto const widget_sizes = children.sizes();

	if(!layout.is_valid())
	{
		for(auto k : children.element_indices())
		{
			if(widget_states[k].collapsed) [[unlikely]]
			{ continue; }

			// If there is no layout, assume that every child can use the entire space
			widget_sizes[k] = run(adjust_cell_sizes_context{children, k}, available_size);
		}
		return ctxt.default_size();
	}

	layout.adjust_cell_widths(available_size[0]);
	layout.adjust_cell_heights(available_size[1]);
	single_array cell_sizes{static_cast<array_size<box_size>>(std::size(children).get())};
	layout.get_cell_sizes_into(cell_sizes);

	for(auto k : children.element_indices())
	{
		if(widget_states[k].collapsed) [[unlikely]]
		{ continue; }

		widget_sizes[k] = run(adjust_cell_sizes_context{children, k}, cell_sizes[array_index<box_size>{k.get()}]);
	}

	return layout.get_dimensions();
}

terraformer::box_size terraformer::ui::main::run(confirm_widget_size_context const& ctxt, box_size size)
{
	auto const new_size = ctxt.confirm_size(size);
	auto const children = ctxt.children();
	auto const widget_states = children.widget_states();
	auto const widget_geometries = children.widget_geometries();
	auto const widget_sizes = children.sizes();

	// TODO: Want to have a permanent array of cell sizes
	single_array cell_sizes{static_cast<array_size<box_size>>(std::size(children).get())};
	auto const layout = ctxt.get_layout();
	if(layout.is_valid())
	{ layout.get_cell_sizes_into(cell_sizes); }

	for(auto k : children.element_indices())
	{
		if(widget_states[k].collapsed) [[unlikely]]
		{ continue; }

		auto size = (layout.is_valid() && widget_states[k].maximized)?
			cell_sizes[array_index<box_size>{k.get()}] : widget_sizes[k];

		widget_geometries[k].size = run(confirm_widget_size_context{children, k}, size);
	}
	return new_size;
}

void terraformer::ui::main::run(update_widget_location_context const& ctxt)
{
	auto const layout = ctxt.get_layout();
	if(!layout.is_valid())
	{ return; }

	auto const children = ctxt.children();

	single_array locs_out(array_size<location>{children.size()});
	layout.get_cell_locations_into(locs_out);

	auto const widget_geometries = children.widget_geometries();
	auto const widget_states = children.widget_states();
	for(auto k : children.element_indices())
	{
		if(widget_states[k].collapsed) [[unlikely]]
		{ continue; }

		widget_geometries[k].where = locs_out[array_index<location>{k.get()}];
			// TODO: Add support for widget alignment within the cell. 0.0f should mean centered.
		widget_geometries[k].origin = location{-1.0f, 1.0f, 0.0f};
		run(update_widget_location_context{children, k});
	}
}

terraformer::ui::main::widget_layer_stack
terraformer::ui::main::run(
	prepare_for_presentation_context const& ctxt,
	graphics_backend_ref backend
)
{
	auto ret = ctxt.prepare_for_presentation(backend);
	auto& children = ctxt.children();
	auto const widget_states = children.widget_states();
	auto const layer_stacks = children.widget_layer_stacks();
	for(auto k : children.element_indices())
	{
		if(widget_states[k].hidden || widget_states[k].collapsed) [[unlikely]]
		{ continue; }

		layer_stacks[k] = run(prepare_for_presentation_context{children, k}, backend);
	}
	return ret;
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
