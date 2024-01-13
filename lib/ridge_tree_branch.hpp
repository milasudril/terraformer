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

		struct seed_info
		{
			array_tuple<location, direction> branch_points;
		};

		auto const& left_seeds() const
		{ return m_left_seeds; }

		auto const& right_seeds() const
		{ return m_right_seeds; }

		auto const& curve() const
		{ return m_points; }

		auto & curve()
		{ return m_points; }

	private:
		array_tuple<location, float> m_points;
		seed_info m_left_seeds;
		seed_info m_right_seeds;
	};
}

#endif