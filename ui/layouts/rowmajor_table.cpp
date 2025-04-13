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

terraformer::scaling terraformer::ui::layouts::rowmajor_table::adjust_cell_sizes(
	struct params const& params,
	state& state,
	scaling available_size
)
{
	single_array<size_t> cols_to_expand;
	auto const colcount = state.m_fixdim_cellcount;
	auto cols = state.m_fixdim_cellsizes.get();
	auto fixed_width = params.no_outer_margin? 0.0f : 2.0f*params.margin_x;
	auto const available_width = available_size[0];
	for(size_t k = 0; k != colcount; ++k)
	{
		using index_type = single_array<column_width>::index_type;
		index_type const index{k};

		fixed_width += std::visit(
			overload{
				[k, cols](column_width::use_default){
					return cols[k];
				},
				[k, &cols_to_expand](column_width::expand){
					cols_to_expand.push_back(k);
					return 0.0f;
				},
				[k, cols, available_width](ratio ratio){
					auto const resulting_height = ratio*available_width;
					cols[k] = resulting_height;
					return resulting_height;
				},
				[k, cols](float value){
					cols[k] = value;
					return value;
				}
			},
			params.column_widths.value_or(index, column_width::use_default{}).value
		);
	}
	if(!cols_to_expand.empty())
	{
		// TODO: Check margins
		auto const remaining_width = available_width - fixed_width;
		auto const avg_col_width = remaining_width/static_cast<float>(std::size(cols_to_expand).get());
		for(auto k : cols_to_expand.element_indices())
		{ cols[cols_to_expand[k]] = avg_col_width; }
	}

	single_array<single_array<float>::index_type> rows_to_expand;
	span<float> rows{state.m_dyndim_cellsizes};
	auto fixed_height = params.no_outer_margin? 0.0f : 2.0f*params.margin_y;
	auto const available_height = available_size[1];
	for(auto k : rows.element_indices())
	{
		using index_type = single_array<row_height>::index_type;
		index_type const index{k.get()};

		fixed_width += std::visit(
			overload{
				[k, rows](row_height::use_default) {
					return rows[k];
				},
				[k, &rows_to_expand](row_height::expand){
					rows_to_expand.push_back(k);
					return 0.0f;
				},
				[k, rows, available_width](ratio ratio){
					auto const resulting_width = ratio*available_width;
					rows[k] = resulting_width;
					return resulting_width;
				},
				[k, rows](float value){
					rows[k] = value;
					return value;
				}
			},
			params.row_heights.value_or(index, row_height::use_default{}).value
		);
	}
	if(!rows_to_expand.empty())
	{
		// TODO: Check margins
		auto const remaining_height = available_height - fixed_height;
		auto const avg_row_height = remaining_height/static_cast<float>(std::size(rows_to_expand).get());
		for(auto k : rows_to_expand.element_indices())
		{ rows[rows_to_expand[k]] = avg_row_height; }
	}

	return scaling{
		std::accumulate(cols, cols + colcount, params.margin_x)
			- (params.no_outer_margin? 2.0f*params.margin_x : 0.0f),
		std::accumulate(
			std::begin(rows),
			std::end(rows),
			params.no_outer_margin?
				0.0f:params.margin_y
		),
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