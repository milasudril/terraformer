//@	{"target": {"name":"table_new.o"}}

#include "./table_new.hpp"

#include "lib/common/utils.hpp"
#include <numeric>

terraformer::ui::layouts::table_new::row_array<float>
terraformer::ui::layouts::table_new::set_default_cell_sizes_to(
	std::span<scaling const> sizes_in,
	column_array<float>& col_widths
)
{
	auto const colcount = std::size(col_widths);
	row_array<float> ret{row_count{ceil_div(std::size(sizes_in), colcount)}};

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

terraformer::ui::layouts::table_new::column_array<float>
terraformer::ui::layouts::table_new::set_default_cell_sizes_to(
	std::span<scaling const> sizes_in,
	row_array<float>& row_heights
)
{
	auto const rowcount = std::size(row_heights);
	column_array<float> ret{column_count{ceil_div(std::size(sizes_in), rowcount)}};

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
			col_width =0.0f;
			++current_col;
		}
	}

	if(current_col != std::size(ret))
	{ ret[current_col] = col_width; }

	return ret;
}

void terraformer::ui::layouts::table_new::set_default_cell_sizes_to(std::span<scaling const> sizes_in)
{
	if(std::size(sizes_in) == 0)
	{ return; }

	switch(m_cell_order)
	{
		case cell_order::row_major:
			m_rows = set_default_cell_sizes_to(sizes_in, m_cols);
			break;

		case cell_order::column_major:
			m_cols = set_default_cell_sizes_to(sizes_in, m_rows);
			break;
	}
}

void terraformer::ui::layouts::table_new::get_cell_sizes_into(
	std::span<scaling> sizes_out,
	row_array<float> const& row_heights,
	column_array<float> const& col_widths
)
{
	size_t colcount = std::size(col_widths);
	size_t current_row = 0;
	size_t current_col = 0;
	for(auto& item : sizes_out)
	{
		item = scaling{col_widths[current_col], row_heights[current_row], 1.0f};
		++current_col;

		if(current_col == colcount)
		{
			current_col = 0;
			++current_row;
		}
	}
}

void terraformer::ui::layouts::table_new::get_cell_sizes_into(
	std::span<scaling> sizes_out,
	column_array<float> const& col_widths,
	row_array<float> const& row_heights
)
{
	auto const rowcount = std::size(row_heights);
	size_t current_row = 0;
	size_t current_col = 0;
	for(auto& item : sizes_out)
	{
		item = scaling{col_widths[current_col], row_heights[current_row], 1.0f};
		++current_row;

		if(current_row == rowcount)
		{
			current_row = 0;
			++current_col;
		}
	}
}

void terraformer::ui::layouts::table_new::get_cell_sizes_into(std::span<scaling> sizes_out) const
{
	switch(m_cell_order)
	{
		case cell_order::row_major:
			get_cell_sizes_into(sizes_out, m_rows, m_cols);
			break;
		case cell_order::column_major:
			get_cell_sizes_into(sizes_out, m_cols, m_rows);
			break;
	}
}

void terraformer::ui::layouts::table_new::get_cell_locations_into(
	std::span<location> locs_out,
	row_array<float> const& row_heights,
	column_array<float> const& col_widths,
	common_params const& params
)
{
	size_t colcount = std::size(col_widths);
	auto loc_x = params.no_outer_margin? 0.0f : params.margin_x;
	auto loc_y = params.no_outer_margin? 0.0f : -params.margin_y;
	size_t current_row = 0;
	size_t current_col = 0;
	for(auto& item : locs_out)
	{
		item = location{loc_x, loc_y, 0.0f},
		loc_x += col_widths[current_col] + params.margin_x;
		++current_col;

		if(current_col == colcount)
		{
			current_col = 0;
			loc_x = params.no_outer_margin? 0.0f : params.margin_x;
			loc_y -= (row_heights[current_row] + params.margin_y);
			++current_row;
		}
	}
}

void terraformer::ui::layouts::table_new::get_cell_locations_into(
	std::span<location> locs_out,
	column_array<float> const& col_widths,
	row_array<float> const& row_heights,
	common_params const& params
)
{
	auto const rowcount = std::size(row_heights);
	auto loc_x = params.no_outer_margin? 0.0f : params.margin_x;
	auto loc_y = params.no_outer_margin? 0.0f : -params.margin_y;
	size_t current_row = 0;
	size_t current_col = 0;
	for(auto& item : locs_out)
	{
		item = location{loc_x, loc_y, 0.0f},
		loc_y -= (row_heights[current_row] + params.margin_y);
		++current_row;

		if(current_row == rowcount)
		{
			current_row = 0;
			loc_y = params.no_outer_margin? 0.0f : -params.margin_y;
			loc_x += col_widths[current_col] + params.margin_x;
			++current_col;
		}
	}
}

void terraformer::ui::layouts::table_new::get_cell_locations_into(std::span<location> locs_out) const
{
	switch(m_cell_order)
	{
		case cell_order::row_major:
			get_cell_locations_into(locs_out, m_rows, m_cols, m_params);
			break;
		case cell_order::column_major:
			get_cell_locations_into(locs_out, m_cols, m_rows, m_params);
			break;
	}
}

terraformer::scaling terraformer::ui::layouts::table_new::get_dimensions() const
{
	auto const width = std::accumulate(std::begin(m_cols), std::end(m_cols), 0.0f)
		+	(static_cast<float>(std::size(m_cols)) + (m_params.no_outer_margin? -1.0f : 1.0f))*m_params.margin_x;
	auto const height = std::accumulate(std::begin(m_rows), std::end(m_rows), 0.0f)
		+	(static_cast<float>(std::size(m_rows)) + (m_params.no_outer_margin? -1.0f : 1.0f))*m_params.margin_y;

	return scaling{width, height, 1.0f};
}