//@	{"target": {"name":"./ridge_tree.o"}}

#include "./ridge_tree.hpp"

terraformer::ridge_tree terraformer::generate_tree(
	std::span<ridge_tree_branch_displacement_description const> curve_levels,
	random_generator
)
{
	ridge_tree ret{};
	if(std::size(curve_levels) == 0)
	{ return ret; }

	return ret;
}