//@	{"dependencies_extra":[{"ref":"./ridge_tree_branch_seed_sequence.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_BRANCH_SEED_SEQUENCE_HPP
#define TERRAFORMER_RIDGE_TREE_BRANCH_SEED_SEQUENCE_HPP

#include "lib/curve_tools/displace.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/array_tuple.hpp"

namespace terraformer
{
	struct ridge_tree_branch_seed_sequence
	{
		array_tuple<location, direction, displaced_curve::index_type> left;
		array_tuple<location, direction, displaced_curve::index_type> right;
	};

	ridge_tree_branch_seed_sequence collect_ridge_tree_branch_seeds(
		displaced_curve const& points
	);

	array_tuple<ridge_tree_branch_seed_sequence, size_t> collect_ridge_tree_branch_seeds(
		std::span<displaced_curve const> points
	);
};

#endif