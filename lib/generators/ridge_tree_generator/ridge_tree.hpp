//@	{"dependencies_extra":[{"ref":"./ridge_tree.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_HPP
#define TERRAFORMER_RIDGE_TREE_HPP

#include "./ridge_curve.hpp"
#include "./ridge_tree_branch.hpp"

#include "lib/curve_tools/displace.hpp"
#include "lib/common/rng.hpp"

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

	private:
		std::vector<ridge_tree_branch_collection> m_value;
	};

	inline ridge_tree generate(
		ridge_tree_description const& description,
		random_generator rng,
		float pixel_size
	)
	{ return ridge_tree{description, rng, pixel_size}; }
}

#endif