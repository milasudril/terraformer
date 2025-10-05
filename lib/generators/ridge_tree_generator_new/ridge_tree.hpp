//@	{"dependencies_extra":[{"ref":"./ridge_tree.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_HPP
#define TERRAFORMER_RIDGE_TREE_HPP

#include "./ridge_curve.hpp"
#include "./ridge_tree_branch.hpp"

#include "lib/array_classes/single_array.hpp"
#include "lib/curve_tools/displace.hpp"
#include "lib/common/rng.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/math_utils/cubic_spline.hpp"

#include <vector>

namespace terraformer
{
	struct ridge_tree_trunk_curve
	{
		cubic_spline_control_point<location, displacement> begin;
		cubic_spline_control_point<location, displacement> end;
	};

	ridge_tree_trunk generate_trunk(
		ridge_tree_trunk_curve const& params,
		ridge_tree_branch_displacement_description const& horz_displacement_profile,
		random_generator& rng
	);
}

#endif