//@	{"dependencies_extra":[{"ref":"./ridge_tree.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_HPP
#define TERRAFORMER_RIDGE_TREE_HPP

#include "./ridge_curve.hpp"

#include "lib/curve_tools/displace.hpp"
#include "lib/common/rng.hpp"

#include <vector>

namespace terraformer
{
	struct ridge_tree_branch
	{
		size_t level;
		displaced_curve curve;
	};

	struct ridge_tree : std::vector<ridge_tree_branch>
	{
		using std::vector<ridge_tree_branch>::vector;
	};

	ridge_tree generate_tree(
		std::span<ridge_tree_branch_displacement_description const> curve_levels,
		random_generator rng
	);
}

#endif