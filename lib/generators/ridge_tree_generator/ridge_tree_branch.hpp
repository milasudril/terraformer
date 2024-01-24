//@	{"dependencies_extra":[{"ref":"./ridge_tree_branch.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_BRANCH_HPP
#define TERRAFORMER_RIDGE_TREE_BRANCH_HPP

#include "./ridge_curve.hpp"
#include "./ridge_tree_branch_seed_sequence.hpp"

#include "lib/common/array_tuple.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/math_utils/differentiation.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include "lib/curve_tools/length.hpp"
#include "lib/curve_tools/displace.hpp"

namespace terraformer
{
	using ridge_tree_branch = array_tuple<location, float>;

	template<class BranchStopCondition>
	std::vector<location> generate_branch_base_curve(
		location loc,
		direction start_dir,
		span_2d<float const> potential,
		float pixel_size,
		BranchStopCondition&& stop)
	{
		std::vector<location> base_curve;
		if(stop(loc) || !inside(potential, loc[0]/pixel_size, loc[1]/pixel_size))
		{ return base_curve; }

		base_curve.push_back(loc);

		loc += pixel_size*start_dir;

		while(!stop(loc) && inside(potential, loc[0]/pixel_size, loc[1]/pixel_size))
		{
			base_curve.push_back(loc);
			auto const g = direction{
				grad(
					potential,
					loc[0]/pixel_size,
					loc[1]/pixel_size,
					1.0f,
					clamp_at_boundary{}
				)
			};

			loc -= pixel_size*g;
		}
		return base_curve;
	}

	void trim_at_intersect(std::vector<ridge_tree_branch>& a, std::vector<ridge_tree_branch>& b, float threshold);

	std::vector<ridge_tree_branch>
	generate_branches(
		array_tuple<location, direction> const& branch_points,
		span_2d<float const> potential,
		float pixel_size,
		ridge_curve_description curve_desc,
		random_generator& rng,
		float d_max,
		std::vector<ridge_tree_branch>&& existing_branches = std::vector<ridge_tree_branch>{});

	struct ridge_tree_stem
	{
		std::vector<terraformer::ridge_tree_branch> left;
		std::vector<terraformer::ridge_tree_branch> right;
	};

	std::vector<ridge_tree_stem>
	generate_branches(
		std::span<ridge_tree_branch_seed_sequence const> parents,
		span_2d<float const> potential,
		float pixel_size,
		ridge_curve_description curve_desc,
		random_generator& rng,
		float max_length
	);

	displacement compute_field(std::span<ridge_tree_branch const> branches, location r, float min_distance);

	float compute_potential(std::span<ridge_tree_branch const> branches, location r, float min_distance);

	void compute_potential(
		span_2d<float> output,
		std::span<ridge_tree_branch const> left_siblings,
		std::span<ridge_tree_branch const> right_siblings,
		float pixel_size);
}

#endif