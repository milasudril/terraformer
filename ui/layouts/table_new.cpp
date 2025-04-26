//@	{"target": {"name":"table_new.o"}}

#include "./table_new.hpp"

#include "lib/common/utils.hpp"

terraformer::ui::layouts::table_new::row_array
terraformer::ui::layouts::table_new::set_cell_sizes_to(
	std::span<scaling const> sizes_in,
	column_array& col_widths
)
{
	auto const colcount = std::size(col_widths);
	row_array ret{row_count{ceil_div(std::size(sizes_in), colcount)}};

	size_t current_row = 0;
	size_t current_col = 0;
	auto row_height = 0.0f;

	for(auto const item : sizes_in)
	{
		col_widths[current_col] = std::max(col_widths[current_col], item[0]);
		row_height = std::max(row_height, item[1]);
		++current_col;

		if(current_col == colcount)
		{
			current_col = 0;
			ret[current_row] = row_height;
			row_height = 0.0f;
			++current_row;
		}
	}

	if(current_row != std::size(ret))
	{ ret[current_row] = row_height; }

	return ret;
}

terraformer::ui::layouts::table_new::column_array
terraformer::ui::layouts::table_new::set_cell_sizes_to(
	std::span<scaling const> sizes_in,
	row_array& row_heights
)
{
	auto const rowcount = std::size(row_heights);
	column_array ret{column_count{ceil_div(std::size(sizes_in), rowcount)}};

	size_t current_row = 0;
	size_t current_col = 0;
	auto col_width = 0.0f;

	for(auto const item : sizes_in)
	{
		row_heights[current_row] = std::max(row_heights[current_row], item[1]);
		col_width = std::max(col_width, item[0]);
		++current_row;

		if(current_row == rowcount)
		{
			current_row = 0;
			ret[current_col] = col_width;
			col_width = 0.0f;
			++current_col;
		}
	}

	if(current_col != std::size(ret))
	{ ret[current_col] = col_width; }

	return ret;
}

void terraformer::ui::layouts::table_new::set_cell_sizes_to(std::span<scaling const> sizes_in)
{
	if(std::size(sizes_in) == 0)
	{ return; }

	switch(m_cell_order)
	{
		case cell_order::row_major:
		{
			m_rows = cell_size_array{row_count{ceil_div(std::size(sizes_in), std::size(m_cols))}};
			break;
		}

		case cell_order::column_major:
		{
			m_cols = cell_size_array{column_count{ceil_div(std::size(sizes_in), std::size(m_rows))}};
			break;
		}
	}
}