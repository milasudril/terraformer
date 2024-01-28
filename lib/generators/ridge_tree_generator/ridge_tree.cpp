//@	{"target": {"name":"./ridge_tree.o"}}

#include "./ridge_tree.hpp"
#include "./ridge_tree_branch.hpp"

namespace terraformer
{
	namespace
	{
		std::vector<location> make_point_array(location start_loc, direction dir, size_t count, float dx)
		{
			std::vector<location> ret(count);
			for(size_t k = 0; k != count; ++k)
			{
				ret[k] = start_loc + static_cast<float>(k)*dx*dir;
			}
			return ret;
		}
	}
}

terraformer::ridge_tree terraformer::generate(
	ridge_tree_description const& description,
	random_generator rng,
	float pixel_size
)
{
	ridge_tree ret{};
	std::span curve_levels{description.curve_levels};

	if(std::size(curve_levels) == 0)
	{ return ret; }

	auto const trunk_pixel_count = static_cast<size_t>(curve_levels[0].growth_params.max_length/pixel_size);
	auto const trunk_offsets = generate(
		curve_levels[0].displacement_profile,
		rng,
		trunk_pixel_count,
		pixel_size,
		1024.0f);

	auto const trunk_base_curve = terraformer::make_point_array
		(description.root_location, description.trunk_direction, trunk_pixel_count, pixel_size);

	ret.push_back(
		ridge_tree_branch_collection{
			.level = 0,
			.curves = std::vector{
				displace_xy(
					trunk_base_curve,
					terraformer::displacement_profile{
						.offsets = trunk_offsets,
						.sample_period = pixel_size
					}
				)
			},
			.parent = ridge_tree_branch_collection::no_parent,
			.side = ridge_tree_branch_collection::side::left
		}
	);

	size_t current_trunk_index = 0;

	while(true)
	{
		if(current_trunk_index == std::size(ret))
		{ return ret; }

		auto const& current_trunk = ret[current_trunk_index];
		auto const next_level_index = current_trunk.level  + 1;
		if(next_level_index == std::size(curve_levels))
		{
			++current_trunk_index;
			continue;
		}

		std::span<displaced_curve const> stem{current_trunk.curves};

		auto const next_level_seeds = terraformer::collect_ridge_tree_branch_seeds(stem);
		auto next_level = generate_branches(
			next_level_seeds,
			ret,
			pixel_size,
			curve_levels[next_level_index].displacement_profile,
			rng,
			curve_levels[next_level_index].growth_params
		);

		for(auto& stem: next_level)
		{
			ret.push_back(
				ridge_tree_branch_collection{
					.level = next_level_index,
					.curves = std::move(stem.left),
					.parent = current_trunk_index,
					.side = ridge_tree_branch_collection::side::left
				}
			);

			ret.push_back(
				ridge_tree_branch_collection{
					.level = next_level_index,
					.curves = std::move(stem.right),
					.parent = current_trunk_index,
					.side = ridge_tree_branch_collection::side::right
				}
			);
		}
		++current_trunk_index;
	}

	return ret;
}