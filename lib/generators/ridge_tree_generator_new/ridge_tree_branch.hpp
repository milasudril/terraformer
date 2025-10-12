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
#include "lib/math_utils/fp_props.hpp"

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

	polynomial<float, 3> create_polynomial(
		float curve_length,
		elevation z_0,
		ridge_tree_ridge_elevation_profile_description const& elevation_profile,
		random_generator& rng
	);

	single_array<float> generate_elevation_profile(
		span<float const, array_index<float>, array_size<float>> integrated_curve_length,
		polynomial<float, 3> const& ridge_polynomial
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
		polynomial<float, 3> const& initial_elevation,
		ridge_tree_elevation_modulation_description const& elevation_profile,
		random_generator& rng
	);



	struct ridge_tree_branch_sequence :
		multi_array<displaced_curve, displaced_curve::index_type, single_array<displaced_curve::index_type>, single_array<float>, float>
	{
		using multi_array<
			displaced_curve,
			displaced_curve::index_type,
			single_array<displaced_curve::index_type>,
			single_array<float>,
			float
		>::multi_array;
	};

	struct ridge_tree_closest_point_curves_result
	{
		curve_distance_result result;
		array_index<displaced_curve> curve{static_cast<size_t>(-1)};
	};

	inline auto closest_point_xy(span<displaced_curve const> curves, location loc)
	{
		if(curves.empty())
		{
			return ridge_tree_closest_point_curves_result{};
		}

		ridge_tree_closest_point_curves_result ret{
			.result = curve_closest_point_xy(curves.front().points(), loc),
			.curve = curves.element_indices().front()
		};

		for(auto k : curves.element_indices(1))
		{
			auto new_res = curve_closest_point_xy(curves[k].points(), loc);
			if(new_res.distance < 0.0f)
			{ continue; }

			if(new_res.distance < ret.result.distance)
			{
				ret.result = new_res;
				ret.curve = k;
			}
		}
		return ret;
	}

	inline auto closest_point_xy(ridge_tree_branch_sequence const& seed_seq, location loc)
	{	return closest_point_xy(seed_seq.get<0>(), loc); }

	using ridge_tree_branch_elevation_data = single_array<polynomial<float, 3>>;

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

	struct ridge_tree_closest_point_result
	{
		ridge_tree_closest_point_curves_result distance_result;
		array_index<ridge_tree_trunk> branch = ridge_tree_trunk::no_parent;
	};

	inline auto closest_point_xy(span<ridge_tree_trunk const> branches, location loc)
	{
		if(branches.empty())
		{
			return ridge_tree_closest_point_result{};
		}

		ridge_tree_closest_point_result ret{
			.distance_result = closest_point_xy(branches.front(), loc),
			.branch = branches.element_indices().front()
		};

		for(auto k : branches.element_indices(1))
		{
			auto const res = closest_point_xy(branches[k], loc);
			if(res.result.distance < 0.0f)
			{ continue; }

			if(res.result.distance < ret.distance_result.result.distance)
			{
				ret.distance_result = res;
				ret.branch = k;
			}
		}

		return ret;
	}

	template<class BranchStopCondition>
	single_array<location> generate_branch_base_curve(
		location loc,
		direction start_dir,
		span_2d<float const> current_heightmap,
		float pixel_size,
		BranchStopCondition&& stop
	)
	{
		assert(pixel_size > 0.0f);
		single_array<location> base_curve;
		if(stop(loc))
		{ return base_curve; }

		base_curve.push_back(loc);
		auto v = (loc - location{})/pixel_size;

		base_curve.reserve(array_size<location>{128});
		auto step = 1.0f*start_dir;
		v += 4.0f*step;
		auto current_elevation = interp(current_heightmap, v[0], v[1], clamp_at_boundary{});

		auto k = base_curve.element_indices().front();
		while(!stop(location{} + v*pixel_size))
		{
			auto const next_elevation =  interp(current_heightmap, v[0], v[1], clamp_at_boundary{});
			if(next_elevation > current_elevation || !inside(current_heightmap, v[0], v[1]))
			{ return base_curve; }

			auto const next_loc = location{} + v*pixel_size;

			if(k > base_curve.element_indices().front() + 1)
			{
				if(distance(base_curve[k - 1], next_loc) <= 0.25f*pixel_size)
				{ return base_curve; }
			}

			base_curve.push_back(next_loc);
			current_elevation = next_elevation;

			auto const dx = 0.5f*(
				   interp(current_heightmap, v[0] + 1.0f, v[1], clamp_at_boundary{})
				 - interp(current_heightmap, v[0] - 1.0f, v[1], clamp_at_boundary{})
			)/pixel_size;
			auto const dy = 0.5f*(
				   interp(current_heightmap, v[0], v[1] + 1.0f, clamp_at_boundary{})
				 - interp(current_heightmap, v[0], v[1] - 1.0f, clamp_at_boundary{})
			)/pixel_size;
			displacement const grad{dx, dy, 0.0f};

			auto const grad_norm = norm(grad);

			step = grad_norm <= 1.0f/64.0f? step : -grad/grad_norm;
			v += 2.0f*step;
			++k;
		}

		return base_curve;
	}

	ridge_tree_branch_sequence
	generate_branches(
		ridge_tree_branch_seed_sequence const& branch_points,
		span_2d<float const> current_heightmap,
		float pixel_size,
		ridge_tree_branch_displacement_description const& curve_desc,
		random_generator& rng,
		float d_max,
		ridge_tree_branch_sequence&& gen_branches = ridge_tree_branch_sequence{});

	void trim_at_intersect(
		ridge_tree_branch_sequence::span_type const& a_seq,
		ridge_tree_branch_sequence::span_type const& b_seq,
		float threshold
	);

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
		float max_length;
		float min_neighbour_distance;
	};

	single_array<ridge_tree_stem_collection>
	generate_branches(
		std::span<ridge_tree_branch_seed_sequence_pair const> parents,
		span_2d<float const> current_heightmap,
		float pixel_size,
		ridge_tree_branch_displacement_description const &curve_desc,
		random_generator& rng,
		ridge_tree_branch_growth_description growth_params
	);
}

#endif