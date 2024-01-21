//@	{"dependencies_extra":[{"ref":"./ridge_tree_branch.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_HPP
#define TERRAFORMER_RIDGE_TREE_HPP

#include "./ridge_tree_branch.hpp"

namespace terraformer
{
	struct ridge_tree
	{
		ridge_tree_branch stem;
		std::vector<ridge_tree> children;
	};

	ridge_tree generate(size_t max_depth);
}

#endif