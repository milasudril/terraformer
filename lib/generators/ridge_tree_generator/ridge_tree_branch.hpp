//@	{"dependencies_extra":[{"ref":"./ridge_tree_branch.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_BRANCH_HPP
#define TERRAFORMER_RIDGE_TREE_BRANCH_HPP

#include "./ridge_curve.hpp"
#include "./ridge_tree_branch_seed_sequence.hpp"

#include "lib/common/spaces.hpp"
#include "lib/modules/dimensions.hpp"
#include "lib/common/rng.hpp"
#include "lib/math_utils/differentiation.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include "lib/math_utils/cubic_spline.hpp"
#include "lib/curve_tools/length.hpp"
#include "lib/curve_tools/displace.hpp"

namespace terraformer
{
	struct per_branch_point_elevation_profile
	{
		modulation_depth peak_modulation_depth;
		slope_angle min_peak_angle;
		slope_angle max_peak_angle;
	};

	single_array<float> gen_per_branch_point_control_points(
		span<location const, array_index<location>, array_size<location>> locations,
		span<array_index<location> const> branch_points,
		per_branch_point_elevation_profile const& params,
		random_generator& rng
	);

	struct ridge_tree_branch_sequence :
		multi_array<displaced_curve, displaced_curve::index_type, single_array<displaced_curve::index_type>>
	{
		using multi_array<
			displaced_curve,
			displaced_curve::index_type,
			single_array<displaced_curve::index_type>
		>::multi_array;
	};

	struct ridge_tree_trunk
	{
		static constexpr auto no_parent = array_index<ridge_tree_trunk>{static_cast<size_t>(-1)};
		enum class side:int{left, right};

		size_t level;
		ridge_tree_branch_sequence branches;
		array_index<ridge_tree_trunk> parent;
		array_index<displaced_curve> parent_curve_index;
		enum side side;
	};

	displacement compute_field(span<displaced_curve const> branches, location r, float min_distance);

	displacement compute_field(span<ridge_tree_trunk const> branches, location r, float min_distance);

	template<class BranchStopCondition>
	single_array<location> generate_branch_base_curve(
		location loc,
		direction start_dir,
		span<ridge_tree_trunk const> trunks,
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
			auto const g = direction{compute_field(trunks, loc, pixel_size)};
			loc -= pixel_size*g;
		}
		return base_curve;
	}

	ridge_tree_branch_sequence
	generate_branches(
		ridge_tree_branch_seed_sequence const& branch_points,
		span<ridge_tree_trunk const> trunks,
		float pixel_size,
		ridge_tree_branch_displacement_description curve_desc,
		random_generator& rng,
		float d_max,
		ridge_tree_branch_sequence&& gen_branches = ridge_tree_branch_sequence{});

	void trim_at_intersect(span<displaced_curve> a, span<displaced_curve> b, float threshold);

	struct ridge_tree_stem_collection
	{
		ridge_tree_stem_collection(array_index<displaced_curve> _parent_curve_index)
			:parent_curve_index{_parent_curve_index}
		{}

		array_index<displaced_curve> parent_curve_index;
		ridge_tree_branch_sequence left;
		ridge_tree_branch_sequence right;
	};

	struct ridge_tree_branch_growth_description
	{
		domain_length max_length;
		domain_length min_neighbour_distance;
	};

	single_array<ridge_tree_stem_collection>
	generate_branches(
		std::span<ridge_tree_branch_seed_sequence_pair const> parents,
		span<ridge_tree_trunk const> trunks,
		float pixel_size,
		ridge_tree_branch_displacement_description curve_desc,
		random_generator& rng,
		ridge_tree_branch_growth_description growth_params
	);
}

#endif