//@	{"target": {"name":"./ridge_tree.o"}}

#include "./ridge_tree.hpp"
#include "./ridge_tree_branch.hpp"

#include "lib/pixel_store/image.hpp"
#include <queue>

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

terraformer::ridge_tree terraformer::generate_tree(
	ridge_tree_description const& description,
	random_generator rng,
	float pixel_size
)
{
	ridge_tree ret{};
	std::span curve_levels{description.curve_levels};

	if(std::size(curve_levels) == 0)
	{ return ret; }

	auto const trunk_pixel_count = static_cast<size_t>(curve_levels[0].max_length/pixel_size);
	auto const trunk_offsets = generate(
		curve_levels[0],
		rng,
		trunk_pixel_count,
		pixel_size,
		1024.0f);

	auto const trunk_base_curve = terraformer::make_point_array
		(description.root_location, description.trunk_direction, trunk_pixel_count, pixel_size);

	ret.push_back(
		ridge_tree_branch{
			.level = 0,
			.curve = displace_xy(
				trunk_base_curve,
				terraformer::displacement_profile{
					.offsets = trunk_offsets,
					.sample_period = pixel_size
				}
			)
		}
	);

	auto const pixel_count = static_cast<uint32_t>(49152.0f/pixel_size);
	terraformer::grayscale_image potential{pixel_count, pixel_count};;

	size_t current_branch_index = 0;

	while(true)
	{
		if(current_branch_index == std::size(ret))
		{ return ret; }

		auto const& current_branch = ret[current_branch_index];
		++current_branch_index;

		auto const current_level = current_branch.level;

		if(current_level == std::size(curve_levels))
		{ return ret; }

		std::span<displaced_curve const> stem{&current_branch.curve, 1};
		compute_potential(potential, stem, std::span<displaced_curve const>{}, pixel_size);

		auto const next_level_seeds = terraformer::collect_ridge_tree_branch_seeds(stem);
		auto const next_level = generate_branches(
			next_level_seeds,
			potential,
			pixel_size,
			curve_levels[current_level],
			rng,
			curve_levels[current_level].max_length
		);

		for(auto const& stem: next_level)
		{
			for(auto const& branch: stem.left)
			{
				ret.push_back(
					ridge_tree_branch{
						.level = current_level + 1,
						.curve = branch
					}
				);
			}

			for(auto const& branch: stem.right)
			{
				ret.push_back(
					ridge_tree_branch{
						.level = current_level + 1,
						.curve = branch
					}
				);
			}
		}
	}

	return ret;
}