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

	private:
		array_tuple<location, float> m_points;
	};
}

#endif