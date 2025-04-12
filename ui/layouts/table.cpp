//@	{"target":{"name":"table.o"}}

#include "./table.hpp"

float terraformer::ui::layouts::table::compute_remaining_size(
	span<cell_size const> specified_sizes,
	span<float const> default_sizes,
	float available_size,
	float margin,
	bool no_outer_margin
)
{
	float remaining_size = available_size - (no_outer_margin? 0.0f : margin);
	for(auto k : default_sizes.element_indices())
	{
		using index_type = single_array<cell_size>::index_type;
		index_type const index{k.get()};

		remaining_size -= std::visit(
			overload{
				[k, default_sizes](cell_size::use_default){
					return default_sizes[k];
				},
				[width = margin](cell_size::expand){
					return width;
				},
				[available_size](ratio ratio){
					return ratio*available_size;
				},
				[](float value){
					return value;
				}
			},
			specified_sizes.value_or(index, cell_size::use_default{}).value
		);
	}

	remaining_size -= (no_outer_margin? 0.0f : margin);
	return remaining_size;

}

terraformer::single_array<terraformer::single_array<float>::index_type>
terraformer::ui::layouts::table::collect_expanding_cells(
	span<cell_size const> specified_sizes,
	size_t total_cell_count
)
{
	terraformer::single_array<single_array<float>::index_type> ret;
	ret.reserve(decltype(ret)::size_type{total_cell_count});

	for(size_t k = 0; k != total_cell_count; ++k)
	{
		using index_type = single_array<cell_size>::index_type;
		index_type const index{k};
		auto val = specified_sizes.value_or(index, cell_size::use_default{});
		if(std::holds_alternative<cell_size::expand>(val.value))
		{ret.push_back(decltype(ret)::value_type{k});}
	}

	return ret;
}