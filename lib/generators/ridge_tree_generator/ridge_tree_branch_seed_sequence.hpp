//@	{"dependencies_extra":[{"ref":"./ridge_tree_branch_seed_sequence.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_BRANCH_SEED_SEQUENCE_HPP
#define TERRAFORMER_RIDGE_TREE_BRANCH_SEED_SEQUENCE_HPP

#include "lib/curve_tools/displace.hpp"
#include "lib/array_classes/single_array.hpp"
#include "lib/common/spaces.hpp"

namespace terraformer
{
	struct ridge_tree_branch_seed_sequence : multi_array<location, direction, displaced_curve::index_type>
	{
		using multi_array<location, direction, displaced_curve::index_type>::multi_array;
	};


	struct ridge_tree_branch_seed_sequence_pair
	{
		ridge_tree_branch_seed_sequence left;
		ridge_tree_branch_seed_sequence right;
	};

	ridge_tree_branch_seed_sequence_pair collect_ridge_tree_branch_seeds(
		displaced_curve const& points
	);

	single_array<ridge_tree_branch_seed_sequence_pair>
	collect_ridge_tree_branch_seeds(span<displaced_curve const> points);

	single_array<displaced_curve::index_type> collect_branch_indices(ridge_tree_branch_seed_sequence_pair const& seq_pair);
};

#endif