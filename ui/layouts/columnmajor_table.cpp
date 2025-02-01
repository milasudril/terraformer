//@	{"target":{"name":"columnmajor_table.o"}}

#include "./columnmajor_table.hpp"
#include "lib/common/utils.hpp"

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

void terraformer::ui::layouts::columnmajor_table::adjust_cell_sizes(
	struct params const& params,
	state& state,
	scaling available_size
)
{
	single_array<size_t> rows_to_expand;
	auto const rowcount = state.m_fixdim_cellcount;
	auto rows = state.m_fixdim_cellsizes.get();
	auto fixed_height = params.no_outer_margin? 0.0f : 2.0f*params.margin_y;
	auto const available_height = available_size[1];
	for(size_t k = 0; k != rowcount; ++k)
	{
		using index_type = single_array<row_height>::index_type;
		index_type const index{k};

		fixed_height += std::visit(
			overload{
				[k, rows](row_height::minimize){
					return rows[k];
				},
				[k, &rows_to_expand](row_height::expand){
					rows_to_expand.push_back(k);
					return 0.0f;
				},
				[k, rows, available_height](ratio ratio){
					auto const resulting_height = ratio*available_height;
					rows[k] = resulting_height;
					return resulting_height;
				},
				[k, rows](float value){
					rows[k] = value;
					return value;
				}
			},
			params.row_heights.value_or(index, row_height::minimize{}).value
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

	// Find all cols to expand
	single_array<single_array<float>::index_type> cols_to_expand;
	span<float> cols{state.m_dyndim_cellsizes};
	auto fixed_width = params.no_outer_margin? 0.0f : 2.0f*params.margin_x;
	auto const available_width = available_size[0];
	for(auto k : cols.element_indices())
	{
		using index_type = single_array<column_width>::index_type;
		index_type const index{k.get()};

		fixed_width += std::visit(
			overload{
				[k, cols](column_width::minimize) {
					return cols[k];
				},
				[k, &cols_to_expand](column_width::expand){
					cols_to_expand.push_back(k);
					return 0.0f;
				},
				[k, cols, available_width](ratio ratio){
					auto const resulting_width = ratio*available_width;
					cols[k] = resulting_width;
					return resulting_width;
				},
				[k, cols](float value){
					cols[k] = value;
					return value;
				}
			},
			params.column_widths.value_or(index, column_width::minimize{}).value
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
}

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
