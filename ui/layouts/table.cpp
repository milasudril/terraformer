//@	{"target":{"name":"table.o"}}

#include "./table.hpp"

float terraformer::ui::layouts::table::update_sizes(
	span<cell_size const> specified_sizes,
	span<float> actual_sizes,
	float available_size,
	float margin,
	bool no_outer_margin
)
{
	float remaining_size = available_size - (no_outer_margin? 0.0f : margin);
	single_array<decltype(actual_sizes)::index_type> sizes_to_expand;
	for(auto k : actual_sizes.element_indices())
	{
		using index_type = single_array<cell_size>::index_type;
		index_type const index{k.get()};

		remaining_size -= std::visit(
			overload{
				[k, actual_sizes](cell_size::use_default){
					return actual_sizes[k];
				},
				[&sizes_to_expand, k, width = margin](cell_size::expand){
					sizes_to_expand.push_back(k);
					return width;
				},
				[k, actual_sizes, available_size](ratio ratio){
					auto const new_size = ratio*available_size;
					actual_sizes[k] = new_size;
					return new_size;
				},
				[k, actual_sizes](float value){
					actual_sizes[k] = value;
					return value;
				}
			},
			specified_sizes.value_or(index, cell_size::use_default{}).value
		);
	}
	remaining_size -= (no_outer_margin? 0.0f : margin);

	for(auto k : sizes_to_expand.element_indices())
	{
		actual_sizes[sizes_to_expand[k]] = remaining_size
			/static_cast<float>(std::size(sizes_to_expand).get());
	}

	return std::accumulate(std::begin(actual_sizes), std::end(actual_sizes), no_outer_margin? 0.0f : margin);
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