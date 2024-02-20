//@	{"dependencies_extra":[{"ref":"./ridge_tree_branch_seed_sequence.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_BRANCH_SEED_SEQUENCE_HPP
#define TERRAFORMER_RIDGE_TREE_BRANCH_SEED_SEQUENCE_HPP

#include "lib/curve_tools/displace.hpp"
#include "lib/common/spaces.hpp"

namespace terraformer
{
	struct ridge_tree_branch_seed_sequence
	{
		multi_array<location, direction, displaced_curve::index_type> left;
		multi_array<location, direction, displaced_curve::index_type> right;
	};

	ridge_tree_branch_seed_sequence collect_ridge_tree_branch_seeds(
		displaced_curve const& points
	);

	terraformer::multi_array<
		terraformer::ridge_tree_branch_seed_sequence,
		terraformer::array_index<terraformer::displaced_curve const>
	>
	collect_ridge_tree_branch_seeds(span<displaced_curve const> points);
};

#endif