//@	{"target":{"name":"columnmajor_table.o"}}

#include "./columnmajor_table.hpp"

terraformer::scaling
terraformer::ui::layouts::columnmajor_table::update_widget_locations(
	common_params const& params,
	state const& state,
	main::widget_collection_ref& widgets
)
{
	auto const widget_geometries = widgets.widget_geometries();
	auto x_offset = params.no_outer_margin? 0.0f : params.margin_x;
	auto y_offset = params.no_outer_margin? 0.0f : params.margin_y;

	auto const* const rows = state.m_fixdim_cellsizes.get();
	size_t row = 0;
	auto const rowcount = state.m_fixdim_cellcount;
	auto col = state.m_dyndim_cellsizes.element_indices().front();
	auto const colcount = std::size(state.m_dyndim_cellsizes);

	for(auto k : widgets.element_indices())
	{
		widget_geometries[k].where = location{
			x_offset,
			-y_offset,
			0.0f,
		};
		widget_geometries[k].origin = location{-1.0f, 1.0f, 0.0f};

		y_offset += rows[row];
		++row;
		if(row == rowcount)
		{
			x_offset += state.m_dyndim_cellsizes[col];
			++col;
			if(col == colcount)
			{ break; }
			row = 0;
			y_offset = params.no_outer_margin? 0.0f : params.margin_y;
		}
	}

	return scaling{
		state.m_width,
		state.m_height,
		1.0f,
	};
}

void terraformer::ui::layouts::columnmajor_table::minimize_cell_sizes(
	common_params const& params,
	state& state,
	main::widget_collection_ref const& widgets
)
{
	auto const sizes = widgets.sizes();
	auto rows = state.m_fixdim_cellsizes.get();
	auto const rowcount = state.m_fixdim_cellcount;
	std::fill_n(rows, rowcount, 0.0f);
	state.m_dyndim_cellsizes.clear();
	auto max_width = 0.0f;
	size_t row = 0;
	auto const indices = widgets.element_indices();
	for(auto k : indices)
	{
		// NOTE: It is assumed that size is 0 if widget is collapsed
		rows[row] = std::max(sizes[k][1] + params.margin_y, rows[row]);
		max_width = std::max(
			sizes[k][0] +	((params.no_outer_margin && k == indices.back())? 0.0f : params.margin_x),
			max_width
		);

		++row;
		if(row == rowcount)
		{
			row = 0;
			state.m_dyndim_cellsizes.push_back(max_width);
			max_width = 0.0f;
		}
	}
	if(row != 0)
	{ state.m_dyndim_cellsizes.push_back(max_width); }

	state.m_width = std::accumulate(
		std::begin(state.m_dyndim_cellsizes),
		std::end(state.m_dyndim_cellsizes),
		params.no_outer_margin?
			0.0f:params.margin_x
	);
	state.m_height = std::accumulate(rows, rows + rowcount, params.margin_y)
		 - (params.no_outer_margin? 2.0f*params.margin_y : 0.0f);
}
