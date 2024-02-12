//@	{"dependencies_extra":[{"ref":"./ridge_tree_branch.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_BRANCH_HPP
#define TERRAFORMER_RIDGE_TREE_BRANCH_HPP

#include "./ridge_curve.hpp"
#include "./ridge_tree_branch_seed_sequence.hpp"

#include "lib/common/array_tuple.hpp"
#include "lib/common/spaces.hpp"
#include "lib/math_utils/differentiation.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include "lib/curve_tools/length.hpp"
#include "lib/curve_tools/displace.hpp"

namespace terraformer
{
	struct ridge_tree_branch_collection
	{
		static constexpr auto no_parent = array_index<ridge_tree_branch_collection>{static_cast<size_t>(-1)};
		enum class side:int{left, right};

		size_t level;
		single_array<displaced_curve> curves;
		array_index<ridge_tree_branch_collection> parent;
		enum side side;
	};

	displacement compute_field(std::span<displaced_curve const> branches, location r, float min_distance);

	displacement compute_field(std::span<ridge_tree_branch_collection const> branches, location r, float min_distance);

	template<class BranchStopCondition>
	single_array<location> generate_branch_base_curve(
		location loc,
		direction start_dir,
		std::span<ridge_tree_branch_collection const> existing_branches,
		float pixel_size,
		BranchStopCondition&& stop)
	{
		single_array<location> base_curve;
		if(stop(loc))
		{ return base_curve; }

		base_curve.push_back(loc);

		base_curve.reserve(array_size<location>{128});

		loc += pixel_size*start_dir;

		while(!stop(loc))
		{
			base_curve.push_back(loc);
			auto const g = direction{compute_field(existing_branches, loc, pixel_size)};
			loc -= pixel_size*g;
		}
		return base_curve;
	}

	single_array<displaced_curve>
	generate_branches(
		array_tuple<location, direction, size_t> const& branch_points,
		std::span<ridge_tree_branch_collection const> existing_branches,
		float pixel_size,
		ridge_tree_branch_displacement_description curve_desc,
		random_generator& rng,
		float d_max,
		single_array<displaced_curve>&& gen_branches = single_array<displaced_curve>{});

	void trim_at_intersect(std::span<displaced_curve> a, std::span<displaced_curve> b, float threshold);

	struct ridge_tree_stem_collection
	{
		single_array<displaced_curve> left;
		single_array<displaced_curve> right;
	};

	struct ridge_tree_branch_growth_description
	{
		domain_length max_length;
		domain_length min_neighbour_distance;
	};

	std::vector<ridge_tree_stem_collection>
	generate_branches(
		std::span<ridge_tree_branch_seed_sequence const> parents,
		std::span<ridge_tree_branch_collection const> existing_branches,
		float pixel_size,
		ridge_tree_branch_displacement_description curve_desc,
		random_generator& rng,
		ridge_tree_branch_growth_description growth_params
	);
}

#endif