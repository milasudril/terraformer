//@	{"dependencies_extra":[{"ref":"./ridge_tree.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_HPP
#define TERRAFORMER_RIDGE_TREE_HPP

#include "./ridge_curve.hpp"
#include "./ridge_tree_branch.hpp"

#include "lib/curve_tools/displace.hpp"
#include "lib/common/rng.hpp"
#include "lib/common/span_2d.hpp"

#include <vector>

namespace terraformer
{
	struct ridge_tree_branch_description
	{
		ridge_tree_branch_displacement_description displacement_profile;
		ridge_tree_branch_growth_description growth_params;
	};

	struct ridge_tree_description
	{
		location root_location;
		direction trunk_direction;
		std::vector<ridge_tree_branch_description> curve_levels;
	};

	struct ridge_tree_elevation_noise
	{

		vertical_amplitude amplitude;
		domain_length wavelength;
		float damping;  // TODO: Should be within ]0, 1[
	};

	struct ridge_tree_branch_elevation_profile
	{
		// envelope
		slope_angle starting_slope;
		elevation final_elevation;
		slope_angle final_slope;

		// noise based on peaks
		vertical_amplitude starting_peak_displacement;
		vertical_amplitude final_peak_displacement;
		slope_angle min_peak_angle;
		slope_angle max_peak_angle;

		// continuos noise
		ridge_tree_elevation_noise noise;
	};

	class ridge_tree
	{
	public:
	  explicit ridge_tree(ridge_tree_description const& description, random_generator rng, float pixel_size);

		operator std::span<ridge_tree_branch_collection const>() const
		{ return m_value; }

		size_t size() const
		{ return std::size(m_value);  }

		auto begin() const
		{ return std::begin(m_value); }

		auto end() const
		{ return std::end(m_value); }

		auto data() const
		{ return std::data(m_value); }

		auto const& operator[](size_t index) const
		{ return m_value[index]; }

		void update_elevations(
			elevation initial_elevation,
			std::span<ridge_tree_branch_elevation_profile const>,
			random_generator,
			float pixel_size
		);

	private:
		std::vector<ridge_tree_branch_collection> m_value;
	};

	inline ridge_tree generate(
		ridge_tree_description const& description,
		random_generator rng,
		float pixel_size
	)
	{ return ridge_tree{description, rng, pixel_size}; }

	struct ridge_tree_branch_render_description
	{
		float peak_elevation;
		float peak_radius;
	};

	struct ridge_tree_render_description
	{
		std::vector<ridge_tree_branch_render_description> curve_levels;
	};

	void render(ridge_tree const& tree,
		span_2d<float> output,
		ridge_tree_render_description const& params,
		float pixel_size);
}

#endif