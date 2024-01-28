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
	struct ridge_tree_branch_collection
	{
		static constexpr auto no_parent = static_cast<size_t>(-1);
		enum class side:int{left, right};

		size_t level;
		std::vector<displaced_curve> curves;
		size_t parent;
		enum side side;
	};

	displacement compute_field(std::span<displaced_curve const> branches, location r, float min_distance);

	displacement compute_field(std::span<ridge_tree_branch_collection const> branches, location r, float min_distance);

	template<class BranchStopCondition>
	std::vector<location> generate_branch_base_curve(
		location loc,
		direction start_dir,
		std::span<ridge_tree_branch_collection const> existing_branches,
		float pixel_size,
		BranchStopCondition&& stop)
	{
		std::vector<location> base_curve;
		if(stop(loc))
		{ return base_curve; }

		base_curve.push_back(loc);

		loc += pixel_size*start_dir;

		while(!stop(loc))
		{
			base_curve.push_back(loc);
			auto const g = direction{compute_field(existing_branches, loc, pixel_size)};
			loc -= pixel_size*g;
		}
		return base_curve;
	}


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

	std::vector<displaced_curve>
	generate_branches(
		array_tuple<location, direction> const& branch_points,
		std::span<ridge_tree_branch_collection const> existing_branches,
		float pixel_size,
		ridge_tree_branch_displacement_description curve_desc,
		random_generator& rng,
		float d_max,
		std::vector<displaced_curve>&& gen_branches = std::vector<displaced_curve>{});

	void trim_at_intersect(std::vector<displaced_curve>& a, std::vector<displaced_curve>& b, float threshold);

	struct ridge_tree_stem_collection
	{
		std::vector<displaced_curve> left;
		std::vector<displaced_curve> right;
	};

	std::vector<ridge_tree_stem_collection>
generate_branches(
	std::span<ridge_tree_branch_seed_sequence const> parents,
	std::span<ridge_tree_branch_collection const> existing_branches,
	float pixel_size,
	ridge_tree_branch_displacement_description curve_desc,
	random_generator& rng,
	float max_length
);

	std::vector<displaced_curve>
	generate_branches(
		array_tuple<location, direction> const& branch_points,
		span_2d<float const> potential,
		float pixel_size,
		ridge_tree_branch_displacement_description curve_desc,
		random_generator& rng,
		float d_max,
		std::vector<displaced_curve>&& existing_branches = std::vector<displaced_curve>{});


	std::vector<ridge_tree_stem_collection>
	generate_branches(
		std::span<ridge_tree_branch_seed_sequence const> parents,
		span_2d<float const> potential,
		float pixel_size,
		ridge_tree_branch_displacement_description curve_desc,
		random_generator& rng,
		float max_length
	);


	float compute_potential(std::span<displaced_curve const> branches, location r, float min_distance);

	void compute_potential(
		span_2d<float> output,
		std::span<displaced_curve const> left_siblings,
		std::span<displaced_curve const> right_siblings,
		float pixel_size);
}

#endif