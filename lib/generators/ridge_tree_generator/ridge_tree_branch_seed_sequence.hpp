//@	{"dependencies_extra":[{"ref":"./ridge_tree_branch_seed_sequence.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_BRANCH_SEED_SEQUENCE_HPP
#define TERRAFORMER_RIDGE_TREE_BRANCH_SEED_SEQUENCE_HPP

#include "lib/curve_tools/displace.hpp"
#include "lib/array_classes/single_array.hpp"
#include "lib/common/spaces.hpp"

namespace terraformer
{
	struct ridge_tree_branch_seed_sequence_pair
	{
		multi_array<location, direction, displaced_curve::index_type> left;
		multi_array<location, direction, displaced_curve::index_type> right;
	};

	ridge_tree_branch_seed_sequence_pair collect_ridge_tree_branch_seeds(
		displaced_curve const& points
	);

	single_array<ridge_tree_branch_seed_sequence_pair>
	collect_ridge_tree_branch_seeds(span<displaced_curve const> points);
};

#endif