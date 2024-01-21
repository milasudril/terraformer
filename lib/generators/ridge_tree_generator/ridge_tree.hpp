#ifndef TERRAFORMER_RIDGE_TREE_HPP
#define TERRAFORMER_RIDGE_TREE_HPP

#include "lib/common/array_tuple.hpp"

namespace terraformer
{
	using ridge_tree_branch = array_tuple<location, float>;

	struct ridge_tree
	{
		ridge_tree_branch stem;
		std::vector<ridge_tree> children;
	};
}

#endif