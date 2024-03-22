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
#include "lib/curve_tools/distance.hpp"

#include <random>

namespace terraformer
{
	struct ridge_tree_slope_angle_range
	{
		slope_angle min;
		slope_angle max;
	};

	inline auto pick(ridge_tree_slope_angle_range range, random_generator& rng)
	{
		std::uniform_real_distribution slope_distribution{0.0f, 1.0f};
		auto const delta = range.max - range.min;
		auto const t = slope_distribution(rng);
		return range.min + t*delta;
	}

	struct ridge_tree_ridge_elevation_profile_description
	{
		ridge_tree_slope_angle_range starting_slope;
		elevation final_elevation;
		ridge_tree_slope_angle_range final_slope;
	};

	polynomial<3> create_polynomial(
		float curve_length,
		elevation z_0,
		ridge_tree_ridge_elevation_profile_description const& elevation_profile,
		random_generator& rng
	);

	single_array<float> generate_elevation_profile(
		span<float const, array_index<float>, array_size<float>> integrated_curve_length,
		polynomial<3> const& ridge_polynomial
	);

	struct ridge_tree_elevation_noise
	{
		domain_length wavelength;
		float damping;  // TODO: Should be within ]0, 1[
	};

	struct ridge_tree_peak_elevation_description
	{
		ridge_tree_slope_angle_range slope;
	};

	struct ridge_tree_elevation_modulation_description
	{
		modulation_depth mod_depth;
		blend_value peak_noise_mix;
		ridge_tree_peak_elevation_description per_peak_modulation;
		ridge_tree_elevation_noise elevation_noise;
	};

	single_array<float> generate_elevation_profile(
		span<float const> integrated_curve_length,
		span<displaced_curve::index_type const> branch_points,
		polynomial<3> const& initial_elevation,
		ridge_tree_elevation_modulation_description const& elevation_profile,
		random_generator& rng
	);



	struct ridge_tree_branch_sequence :
		multi_array<displaced_curve, displaced_curve::index_type, single_array<displaced_curve::index_type>, single_array<float>>
	{
		using multi_array<
			displaced_curve,
			displaced_curve::index_type,
			single_array<displaced_curve::index_type>,
			single_array<float>
		>::multi_array;
	};

	inline auto closest_point_xy(span<displaced_curve const> curves, location loc)
	{
		if(curves.empty())
		{
			return curve_distance_result{
				.loc = location{0.0f, 0.0f, 0.0f},
				.distance = -1.0f
			};
		}

		auto result = curve_closest_point_xy(curves.front().points(), loc);
		for(auto k = curves.first_element_index() + 1; k != std::size(curves); ++k)
		{
			auto new_res = curve_closest_point_xy(curves[k].points(), loc);
			if(new_res.distance == -1.0f)
			{ continue; }

			if(new_res.distance < result.distance)
			{ result = new_res; }
		}
		return result;
	}

	inline auto closest_point_xy(ridge_tree_branch_sequence const& seed_seq, location loc)
	{	return closest_point_xy(seed_seq.get<0>(), loc); }

	using ridge_tree_branch_elevation_data = single_array<polynomial<3>>;

	struct ridge_tree_trunk
	{
		static constexpr auto no_parent = array_index<ridge_tree_trunk>{static_cast<size_t>(-1)};
		enum class side:int{left, right};

		size_t level;
		ridge_tree_branch_sequence branches;
		array_index<ridge_tree_trunk> parent;
		array_index<displaced_curve> parent_curve_index;
		enum side side;

		ridge_tree_branch_elevation_data elevation_data;
	};

	displacement compute_field(span<displaced_curve const> branches, location r, float min_distance);

	displacement compute_field(span<ridge_tree_trunk const> branches, location r, float min_distance);

	inline auto closest_point_xy(ridge_tree_trunk const& trunk, location loc)
	{	return closest_point_xy(trunk.branches, loc); }

	struct ridge_tree_closest_point_info
	{
		curve_distance_result distance_result;
		size_t level;
	};

	inline ridge_tree_closest_point_info closest_point_xy(span<ridge_tree_trunk const> branches, location loc)
	{
		if(branches.empty())
		{
			return ridge_tree_closest_point_info{
				.distance_result{
					.loc = loc,
					.distance = -1.0f
				},
				.level = static_cast<size_t>(-1)
			};
		}

		ridge_tree_closest_point_info ret{
			.distance_result = closest_point_xy(branches.front(), loc),
			.level = branches.front().level
		};

		for(auto k = branches.first_element_index() + 1; k != std::size(branches); ++k)
		{
			auto const res = closest_point_xy(branches[k], loc);
			if(res.distance == -1.0f)
			{ continue; }

			if(res.distance < ret.distance_result.distance)
			{
				ret.distance_result = res;
				ret.level = branches[k].level;
			}
		}

		return ret;
	}

	template<class BranchStopCondition>
	single_array<location> generate_branch_base_curve(
		location loc,
		direction start_dir,
		span<ridge_tree_trunk const> trunks,
		float pixel_size,
		BranchStopCondition&& stop
	)
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
		ridge_tree_branch_displacement_description const& curve_desc,
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
		ridge_tree_branch_displacement_description const &curve_desc,
		random_generator& rng,
		ridge_tree_branch_growth_description growth_params
	);
}

#endif