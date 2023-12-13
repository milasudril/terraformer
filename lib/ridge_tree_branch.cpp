//@	{"target":{"name":"ridge_tree_branch.o"}}

#include "./ridge_tree_branch.hpp"

terraformer::ridge_tree_branch::ridge_tree_branch(
	std::span<location const> base_curve,
	displacement_profile dy):
	m_points{displace(base_curve, dy, displacement{0.0f, 0.0f, -1.0f})}
{}