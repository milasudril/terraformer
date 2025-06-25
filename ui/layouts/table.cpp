//@	{"target": {"name":"table.o"}}

#include "./table.hpp"

#include "lib/common/utils.hpp"
#include "lib/array_classes/single_array.hpp"

#include <numeric>
#include <variant>

terraformer::ui::layouts::table::row_array<float>
terraformer::ui::layouts::table::set_default_cell_sizes_to(
	span<box_size const> sizes_in,
	column_array<float>& col_widths
)
{
	auto const colcount = std::size(col_widths);
	row_array<float> ret{row_count{ceil_div(std::size(sizes_in).get(), colcount)}};

	std::ranges::fill(col_widths, 0.0f);
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

terraformer::ui::layouts::table::column_array<float>
terraformer::ui::layouts::table::set_default_cell_sizes_to(
	span<box_size const> sizes_in,
	row_array<float>& row_heights
)
{
	auto const rowcount = std::size(row_heights);
	column_array<float> ret{column_count{ceil_div(std::size(sizes_in).get(), rowcount)}};

	std::ranges::fill(row_heights, 0.0f);
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

void terraformer::ui::layouts::table::set_default_cell_sizes_to(span<box_size const> sizes_in)
{
	if(sizes_in.empty())
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

void terraformer::ui::layouts::table::adjust_cell_widths_by_row(
	float available_width,
	span<float const> size_overrides
)
{
	span<float> const actual_sizes{m_cols};
	auto size_of_fixed_cols = 0.0f;
	auto const col_count = std::size(m_rows);
	array_index<float> current_col{};
	auto greatest_col_width = 0.0f;
	for(auto k: size_overrides.element_indices())
	{
		array_index<cell_size> const cell_size_index{k.get()};
		auto const current_cell_width = std::holds_alternative<cell_size::use_default>(
			m_cell_sizes.value_or(
				cell_size_index, cell_size{}
			).value
		)?
		actual_sizes[current_col] : 0.0f;

		greatest_col_width = std::max(current_cell_width, greatest_col_width);

		++current_col;
		if(current_col.get() == col_count)
		{
			size_of_fixed_cols += greatest_col_width;
			current_col = array_index<float>{};
			greatest_col_width = 0.0f;
		}
	}

	auto const cells_to_expand = get_cells_to_expand(m_cell_sizes, size_overrides);
	auto const no_outer_margin = m_params.no_outer_margin;
	auto const margin = m_params.margin_x;
	auto const size_of_margins = margin*static_cast<float>(
		std::size(actual_sizes).get() + (no_outer_margin? -1 : 1)
	);

	auto const space_for_expanding_cells = available_width - size_of_margins - size_of_fixed_cols;

	auto const num_to_expand = static_cast<float>(std::size(cells_to_expand).get());
	for(auto k : cells_to_expand.element_indices())
	{ actual_sizes[cells_to_expand[k]] = space_for_expanding_cells/num_to_expand; }
}

void terraformer::ui::layouts::table::adjust_cell_sizes_regular(
	span<cell_size const> specified_sizes,
	span<float> actual_sizes,
	float margin,
	bool no_outer_margin,
	float available_size,
	span<float const> size_overrides
)
{
	auto size_of_fixedsized_elements = 0.0f;
	array_index<float> current_item{};
	auto greatest_size = 0.0f;
	auto const stride = std::size(actual_sizes);
	auto has_expanding_cells = false;
	single_array<array_index<float>> items_to_expand;

	for(auto k: size_overrides.element_indices())
	{
		array_index<cell_size> const cell_size_index{k.get()};

		auto const current_cell_size = std::visit(
			overload{
				[default_size = actual_sizes[current_item]](cell_size::use_default){
					return default_size;
				},
				[&has_expanding_cells, overridden_size = size_overrides[k]](cell_size::expand){
					if(overridden_size >= 0.0f)
					{ return overridden_size; }
					has_expanding_cells = true;
					return 0.0f;
				}
			},
			specified_sizes.value_or(cell_size_index, cell_size{}).value
		);

		greatest_size = std::max(current_cell_size, greatest_size);

		++current_item;
		if(current_item == stride)
		{
			size_of_fixedsized_elements += greatest_size;
			if(has_expanding_cells)
			{
				items_to_expand.push_back(current_item);
				has_expanding_cells = false;
			}
			current_item = array_index<float>{};
			greatest_size = 0.0f;
		}
	}

	auto const size_of_margins = margin*static_cast<float>(
		std::size(actual_sizes).get() + (no_outer_margin? -1 : 1)
	);

	auto const space_for_expanding_cells = available_size - size_of_margins - size_of_fixedsized_elements;

	auto const num_to_expand = static_cast<float>(std::size(items_to_expand).get());
	for(auto k : items_to_expand.element_indices())
	{ actual_sizes[items_to_expand[k]] = space_for_expanding_cells/num_to_expand; }
}

void terraformer::ui::layouts::table::adjust_cell_sizes_transposed(
	span<cell_size const> specified_sizes,
	span<float> actual_sizes,
	float margin,
	bool no_outer_margin,
	float available_size,
	span<float const> size_overrides
)
{
	auto const stride = std::size(actual_sizes);
	array_index<float> current_item{};
	single_array<float> collected_sizes{stride};
	auto has_expanding_cells = false;
	single_array<array_index<float>> items_to_expand;
	for(auto k: size_overrides.element_indices())
	{
		array_index<cell_size> const cell_size_index{k.get()};
		auto const current_cell_size = std::visit(
			overload{
				[default_size = actual_sizes[current_item]](cell_size::use_default){
					return default_size;
				},
				[&has_expanding_cells, overridden_size = size_overrides[k]](cell_size::expand){
					if(overridden_size >= 0.0f)
					{ return overridden_size; }
					has_expanding_cells = true;
					return 0.0f;
				}
			},
			specified_sizes.value_or(cell_size_index, cell_size{}).value
		);

		collected_sizes[current_item] = std::max(current_cell_size, collected_sizes[current_item]);

		++current_item;;
		if(current_item == stride)
		{
			if(has_expanding_cells)
			{
				items_to_expand.push_back(current_item);
				has_expanding_cells = false;
			}
			current_item = array_index<float>{};
		}
	}

	auto const size_of_fixedsized_elements = std::accumulate(
		std::begin(collected_sizes),
		std::end(collected_sizes),
		0.0f
	);

	auto const size_of_margins = margin*static_cast<float>(
		std::size(actual_sizes).get() + (no_outer_margin? -1 : 1)
	);

	auto const space_for_expanding_cells = available_size - size_of_margins - size_of_fixedsized_elements;

	auto const num_to_expand = static_cast<float>(std::size(items_to_expand).get());
	for(auto k : items_to_expand.element_indices())
	{ actual_sizes[items_to_expand[k]] = space_for_expanding_cells/num_to_expand; }
}

void terraformer::ui::layouts::table::adjust_cell_sizes(
	span<cell_size const> specified_sizes,
	span<float> actual_sizes,
	float available_size,
	float margin,
	bool no_outer_margin
)
{
	auto size_of_fixed_cells = 0.0f;
	single_array<decltype(actual_sizes)::index_type> cells_to_expand;
	for(auto k : actual_sizes.element_indices())
	{
		using index_type = single_array<cell_size>::index_type;
		index_type const index{k.get()};

		size_of_fixed_cells += std::visit(
			overload{
				[k, actual_sizes](cell_size::use_default){
					return actual_sizes[k];
				},
				[&cells_to_expand, k](cell_size::expand){
					cells_to_expand.push_back(k);
					return 0.0f;
				}
			},
			specified_sizes.value_or(index, cell_size::use_default{}).value
		);
	}

	if(cells_to_expand.empty())
	{ return; }

	auto const size_of_margins = margin*static_cast<float>(
		std::size(actual_sizes).get() + (no_outer_margin? -1 : 1)
	);

	auto const space_for_expanding_cells = available_size - size_of_margins - size_of_fixed_cells;

	auto const num_to_expand = static_cast<float>(std::size(cells_to_expand).get());
	for(auto k : cells_to_expand.element_indices())
	{ actual_sizes[cells_to_expand[k]] = space_for_expanding_cells/num_to_expand; }
}

void terraformer::ui::layouts::table::get_cell_sizes_into(
	span<box_size> sizes_out,
	row_array<float> const& row_heights,
	column_array<float> const& col_widths
)
{
	size_t colcount = std::size(col_widths);
	size_t current_row = 0;
	size_t current_col = 0;
	for(auto& item : sizes_out)
	{
		item = box_size{col_widths[current_col], row_heights[current_row], 1.0f};
		++current_col;

		if(current_col == colcount)
		{
			current_col = 0;
			++current_row;
		}
	}
}

void terraformer::ui::layouts::table::get_cell_sizes_into(
	span<box_size> sizes_out,
	column_array<float> const& col_widths,
	row_array<float> const& row_heights
)
{
	auto const rowcount = std::size(row_heights);
	size_t current_row = 0;
	size_t current_col = 0;
	for(auto& item : sizes_out)
	{
		item = box_size{col_widths[current_col], row_heights[current_row], 1.0f};
		++current_row;

		if(current_row == rowcount)
		{
			current_row = 0;

			++current_col;
		}
	}
}

void terraformer::ui::layouts::table::get_cell_sizes_into(span<box_size> sizes_out) const
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

terraformer::single_array<terraformer::array_index<float>>
terraformer::ui::layouts::table::get_cells_to_expand(
	span<cell_size const> cell_sizes,
	span<float const> size_overrides
)
{
	single_array<array_index<float>> ret;
	for(auto k : size_overrides.element_indices())
	{
		array_index<cell_size> const cell_size_index{k.get()};
		if(
			std::holds_alternative<cell_size::expand>(
				cell_sizes.value_or(cell_size_index, cell_size{}).value
			) &&
			size_overrides.value_or(k, -1.0f) < 0.0f
		)
		{ ret.push_back(k); }
	}
	return ret;
}


void terraformer::ui::layouts::table::get_cell_locations_into(
	span<location> locs_out,
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

void terraformer::ui::layouts::table::get_cell_locations_into(
	span<location> locs_out,
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

void terraformer::ui::layouts::table::get_cell_locations_into(span<location> locs_out) const
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

terraformer::box_size terraformer::ui::layouts::table::get_dimensions() const
{
	auto const width = std::accumulate(std::begin(m_cols), std::end(m_cols), 0.0f)
		+	(static_cast<float>(std::size(m_cols)) + (m_params.no_outer_margin? -1.0f : 1.0f))*m_params.margin_x;
	auto const height = std::accumulate(std::begin(m_rows), std::end(m_rows), 0.0f)
		+	(static_cast<float>(std::size(m_rows)) + (m_params.no_outer_margin? -1.0f : 1.0f))*m_params.margin_y;

	return box_size{std::max(width, m_params.margin_x), std::max(height, m_params.margin_y), 1.0f};
}
