//@	{"dependencies_extra":[{"ref":"./ridge_tree.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_HPP
#define TERRAFORMER_RIDGE_TREE_HPP

#include "./ridge_curve.hpp"

#include "lib/curve_tools/displace.hpp"
#include "lib/common/rng.hpp"

#include <vector>

namespace terraformer
{
	struct ridge_tree_branch_collection
	{
		static constexpr auto no_parent = static_cast<size_t>(-1);

		size_t level;
		std::vector<displaced_curve> curves;
		size_t parent;
	};

	struct ridge_tree : std::vector<ridge_tree_branch_collection>
	{
		using std::vector<ridge_tree_branch_collection>::vector;
	};

	struct ridge_tree_branch_description
	{
		ridge_tree_branch_displacement_description displacement_profile;
		domain_length max_length;
	};

	struct ridge_tree_description
	{
		location root_location;
		direction trunk_direction;
		std::vector<ridge_tree_branch_description> curve_levels;
	};

	ridge_tree generate(
		ridge_tree_description const& description,
		random_generator rng,
		float pixel_size
	);
}

#endif