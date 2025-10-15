//@	{"dependencies_extra":[{"ref":"./ridge_tree_branch.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_BRANCH_HPP
#define TERRAFORMER_RIDGE_TREE_BRANCH_HPP

#include "./ridge_curve.hpp"
#include "./ridge_tree_branch_seed_sequence.hpp"

#include "lib/array_classes/single_array.hpp"
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
		multi_array<displaced_curve, float, displaced_curve::index_type, single_array<displaced_curve::index_type>>
	{
		using multi_array<
			displaced_curve,
			float,
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

	struct ridge_tree_branch_base_curve
	{
		single_array<location> locations;
		float initial_height = 1.0f;
	};

	template<class BranchStopCondition>
	ridge_tree_branch_base_curve
	generate_branch_base_curve(
		location loc,
		direction start_dir,
		span_2d<float const> current_heightmap,
		float pixel_size,
		BranchStopCondition&& stop
	)
	{
		assert(pixel_size > 0.0f);
		ridge_tree_branch_base_curve base_curve{};
		if(stop(loc))
		{ return base_curve; }

		base_curve.locations.push_back(loc);
		auto v = (loc - location{})/pixel_size;
		base_curve.initial_height = interp(current_heightmap, v[0], v[1], clamp_at_boundary{});

		base_curve.locations.reserve(array_size<location>{128});
		auto step = 1.0f*start_dir;
		v += 4.0f*step;
		auto current_elevation = interp(current_heightmap, v[0], v[1], clamp_at_boundary{});

		auto k = base_curve.locations.element_indices().front();
		while(!stop(location{} + v*pixel_size))
		{
			auto const next_elevation =  interp(current_heightmap, v[0], v[1], clamp_at_boundary{});
			if(next_elevation > current_elevation || !inside(current_heightmap, v[0], v[1]))
			{ return base_curve; }

			auto const next_loc = location{} + v*pixel_size;

			if(k > base_curve.locations.element_indices().front() + 1)
			{
				if(distance(base_curve.locations[k - 1], next_loc) <= 0.25f*pixel_size)
				{ return base_curve; }
			}

			base_curve.locations.push_back(next_loc);
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
		ridge_tree_branch_sequence&& gen_branches = ridge_tree_branch_sequence{}
	);

	struct trim_params
	{
		span<displaced_curve> curves;
		span<float const> collision_margins;
	};

	void trim_at_intersect(trim_params const& a_params, trim_params const& b_params);


	struct ridge_tree_stem_collection
	{
		ridge_tree_stem_collection(array_index<displaced_curve> _parent_curve_index)
			:parent_curve_index{_parent_curve_index}
		{}

		array_index<displaced_curve> parent_curve_index;
		ridge_tree_branch_sequence left;
		ridge_tree_branch_sequence right;
	};

	void trim_at_intersct(
		span<ridge_tree_stem_collection> stem_collections,
		std::span<ridge_tree_branch_seed_sequence_pair const> parents
	);

	struct ridge_tree_branch_growth_description
	{
		float max_length;
	};

	single_array<ridge_tree_stem_collection>
	generate_branches(
		std::span<ridge_tree_branch_seed_sequence_pair const> parents,
		span_2d<float const> current_heightmap,
		float pixel_size,
		ridge_tree_branch_displacement_description const& curve_desc,
		random_generator& rng,
		ridge_tree_branch_growth_description growth_params
	);

	single_array<ridge_tree_stem_collection>
	generate_and_prune_branches(
		std::span<ridge_tree_branch_seed_sequence_pair const> parents,
		span_2d<float const> current_heightmap,
		float pixel_size,
		ridge_tree_branch_displacement_description const &curve_desc,
		random_generator& rng,
		ridge_tree_branch_growth_description growth_params
	);
}

#endif