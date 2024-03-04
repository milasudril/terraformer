//@	{"dependencies_extra":[{"ref":"./ridge_tree.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_HPP
#define TERRAFORMER_RIDGE_TREE_HPP

#include "./ridge_curve.hpp"
#include "./ridge_tree_branch.hpp"

#include "lib/array_classes/single_array.hpp"
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

	struct ridge_tree_xy_description
	{
		location root_location;
		direction trunk_direction;
		std::vector<ridge_tree_branch_description> curve_levels;
	};

	struct ridge_tree_branch_elevation_profile
	{
		ridge_tree_ridge_elevation_profile_description base_elevation;
		ridge_tree_elevation_modulation_description modulation;
	};

	class ridge_tree
	{
	public:
	  explicit ridge_tree(ridge_tree_xy_description const& description, random_generator rng, float pixel_size);

		operator std::span<ridge_tree_trunk const>() const
		{ return m_value; }

		constexpr auto first_element_index()
		{ return m_value.first_element_index(); }

		auto size() const
		{ return std::size(m_value);  }

		auto begin() const
		{ return std::begin(m_value); }

		auto end() const
		{ return std::end(m_value); }

		auto data() const
		{ return std::data(m_value); }

		auto const& operator[](array_index<ridge_tree_trunk> index) const
		{ return m_value[index]; }

		void update_elevations(
			elevation initial_elevation,
			std::span<ridge_tree_branch_elevation_profile const> elevation_profiles,
			random_generator rng
		);

	private:
		single_array<ridge_tree_trunk> m_value;
	};

	inline ridge_tree generate(
		ridge_tree_xy_description const& description,
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