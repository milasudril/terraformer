//@	{"target":{"name":"rowmajor_table.o"}}

#include "./rowmajor_table.hpp"

terraformer::scaling terraformer::ui::layouts::rowmajor_table::set_default_cell_sizes(
	common_params const& params,
	state& state,
	main::widget_collection_view const& widgets
)
{
	auto const sizes = widgets.sizes();
	auto cols = state.m_fixdim_cellsizes.get();
	auto const colcount = state.m_fixdim_cellcount;
	std::fill_n(cols, colcount, 0.0f);
	state.m_dyndim_cellsizes.clear();
	auto max_height = 0.0f;
	size_t col = 0;
	auto const indices = widgets.element_indices();
	for(auto k : indices)
	{
		// NOTE: It is assumed that size is 0 if widget is collapsed
		cols[col] = std::max(sizes[k][0] + params.margin_x, cols[col]);
		max_height = std::max(
			sizes[k][1] + ((params.no_outer_margin && k == indices.back())?0.0f:params.margin_y),
			max_height
		);

		++col;
		if(col == colcount)
		{
			col = 0;
			state.m_dyndim_cellsizes.push_back(max_height);
			max_height = 0.0f;
		}
	}

	if(col != 0)
	{ state.m_dyndim_cellsizes.push_back(max_height); }

	return scaling{
		std::accumulate(cols, cols + colcount, params.margin_x)
			- (params.no_outer_margin? 2.0f*params.margin_x : 0.0f),
		std::accumulate(
			std::begin(state.m_dyndim_cellsizes),
			std::end(state.m_dyndim_cellsizes),
			params.no_outer_margin? 0.0f : params.margin_y
		),
		1.0f
	};
}

terraformer::scaling terraformer::ui::layouts::rowmajor_table::get_widget_sizes_into(
	struct params2 const& params,
	state& state,
	scaling available_size,
	main::widget_collection_ref const& widgets
)
{
	auto const width = update_sizes(
		params.column_widths,
		span{
			state.m_fixdim_cellsizes.get(),
			state.m_fixdim_cellsizes.get() + state.m_fixdim_cellcount
		},
		available_size[0],
		params.margin_x,
		params.no_outer_margin
	);

	auto const height = update_sizes(
		params.row_heights,
		state.m_dyndim_cellsizes,
		available_size[1],
		params.margin_y,
		params.no_outer_margin
	);

	auto const widget_states = widgets.widget_states();
	auto const widget_sizes = widgets.sizes();
	auto const cols = state.m_fixdim_cellsizes.get();
	auto const& rows = state.m_dyndim_cellsizes;
	auto const colcount = state.m_fixdim_cellcount;
	auto const rowcount = std::size(state.m_dyndim_cellsizes);
	auto row = state.m_dyndim_cellsizes.element_indices().front();
	size_t col = 0;

	for(auto k : widgets.element_indices())
	{
		if(widget_states[k].maximized)
		{ widget_sizes[k] = scaling{cols[col], rows[row], 1.0f}; }

		if(col == colcount)
		{
			++row;
			if(row == rowcount)
			{ break; }
			col = 0;
		}
	}

	return scaling{
		width,
		height,
		1.0f
	};
}

void
terraformer::ui::layouts::rowmajor_table::update_widget_locations(
	common_params const& params,
	state const& state,
	main::widget_collection_ref& widgets
)
{
	auto const widget_geometries = widgets.widget_geometries();
	auto x_offset = params.no_outer_margin? 0.0f : params.margin_x;
	auto y_offset = params.no_outer_margin? 0.0f : params.margin_y;

	auto const* const cols = state.m_fixdim_cellsizes.get();
	size_t col = 0;
	auto const colcount = state.m_fixdim_cellcount;
	auto row = state.m_dyndim_cellsizes.element_indices().front();
	auto const rowcount = std::size(state.m_dyndim_cellsizes);

	for(auto k : widgets.element_indices())
	{
		widget_geometries[k].where = location{
			x_offset,
			-y_offset,
			0.0f,
		};
		widget_geometries[k].origin = location{-1.0f, 1.0f, 0.0f};

		x_offset += cols[col];
		++col;
		if(col == colcount)
		{
			y_offset += state.m_dyndim_cellsizes[row];
			++row;
			if(row == rowcount)
			{ break; }
			col = 0;
			x_offset = params.no_outer_margin? 0.0f : params.margin_x;
		}
	}
}