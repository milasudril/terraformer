//@	{"target":{"name":"table.o"}}

#include "./table.hpp"

float terraformer::ui::layouts::table::compute_remaining_width(
	struct params const& params,
	span<float const> cell_widths,
	float available_width
)
{
	auto const margin = params.margin_x;
	float remaining_width = available_width - (params.no_outer_margin? 0.0f : margin);
	for(auto k : cell_widths.element_indices())
	{
		using index_type = single_array<column_width>::index_type;
		index_type const index{k.get()};

		remaining_width -= std::visit(
			overload{
				[k, cell_widths](column_width::use_default){
					return cell_widths[k];
				},
				[width = margin](column_width::expand){
					return width;
				},
				[available_width](ratio ratio){
					return ratio*available_width;
				},
				[](float value){
					return value;
				}
			},
			params.column_widths.value_or(index, column_width::use_default{}).value
		);
	}

	remaining_width -= (params.no_outer_margin? 0.0f : params.margin_x);
	return remaining_width;
}

float terraformer::ui::layouts::table::compute_remaining_height(
	struct params const& params,
	span<float const> cell_heights,
	float available_height
)
{
	auto const margin = params.margin_y;
	float remaining_height = available_height - (params.no_outer_margin? 0.0f : margin);
	for(auto k : cell_heights.element_indices())
	{
		using index_type = single_array<row_height>::index_type;
		index_type const index{k.get()};

		remaining_height -= std::visit(
			overload{
				[k, cell_heights](row_height::use_default){
					return cell_heights[k];
				},
				[height = margin](row_height::expand){
					return height;
				},
				[available_height](ratio ratio){
					return ratio*available_height;
				},
				[](float value){
					return value;
				}
			},
			params.row_heights.value_or(index, row_height::use_default{}).value
		);
	}

	remaining_height -= (params.no_outer_margin? 0.0f : margin);
	return remaining_height;
}