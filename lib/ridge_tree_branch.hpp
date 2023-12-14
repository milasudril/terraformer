//@	{"dependencies_extra":[{"ref":"./ridge_tree_branch.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_BRANCH_HPP
#define TERRAFORMER_RIDGE_TREE_BRANCH_HPP

#include "./curve_displace.hpp"

#include <vector>
#include <span>

namespace terraformer
{
	class ridge_tree_branch
	{
	public:
		explicit ridge_tree_branch(std::span<location const> base_curve, displacement_profile dy);

		auto const& left_seeds() const
		{ return m_left_seeds; }

		auto const& right_seeds() const
		{ return m_right_seeds; }

		auto const& curve() const
		{ return m_points; }

	private:
		array_tuple<location, float> m_points;
		array_tuple<location, direction> m_left_seeds;
		array_tuple<location, direction> m_right_seeds;
	};
}

#endif