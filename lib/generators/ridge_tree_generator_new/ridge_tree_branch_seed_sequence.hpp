//@	{"dependencies_extra":[{"ref":"./ridge_tree_branch_seed_sequence.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_BRANCH_SEED_SEQUENCE_HPP
#define TERRAFORMER_RIDGE_TREE_BRANCH_SEED_SEQUENCE_HPP

#include "lib/curve_tools/displace.hpp"
#include "lib/array_classes/single_array.hpp"
#include "lib/common/spaces.hpp"
#include <geosimd/angle.hpp>

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

	struct ridge_tree_brach_seed_sequence_boundary_point_descriptor
	{
		size_t branch_count = 0;
		geosimd::turn_angle spread_angle{geosimd::turns{0.5f}};
	};

	struct ridge_tree_branch_seed_collection_descriptor
	{
		ridge_tree_brach_seed_sequence_boundary_point_descriptor start_branches;
		ridge_tree_brach_seed_sequence_boundary_point_descriptor end_brancehs;
	};

	ridge_tree_branch_seed_sequence_pair collect_ridge_tree_branch_seeds(
		displaced_curve const& points,
		ridge_tree_branch_seed_collection_descriptor const& params
	);

	single_array<ridge_tree_branch_seed_sequence_pair>
	collect_ridge_tree_branch_seeds(
		span<displaced_curve const> points,
		ridge_tree_branch_seed_collection_descriptor const& params
	);

	single_array<displaced_curve::index_type> collect_branch_indices(ridge_tree_branch_seed_sequence_pair const& seq_pair);
};

#endif