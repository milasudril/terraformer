//@	{"dependencies_extra":[{"ref":"./ridge_tree_branch.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_HPP
#define TERRAFORMER_RIDGE_TREE_HPP

#include "./ridge_tree_branch.hpp"

#include <queue>

namespace terraformer
{
	struct ridge_tree
	{
		ridge_tree_branch stem;
		std::vector<ridge_tree> left_children;
		std::vector<ridge_tree> right_children;
	};

	template<class Function, class ... Args>
	void for_each_bfs(ridge_tree const& tree, Function&& f, Args... args)
	{
		std::queue<ridge_tree const*> nodes;
		nodes.push(&tree);
		while(!nodes.empty())
		{
			auto next = nodes.front();
			if(next != nullptr)
			{
				f(*next, args...);

				for(auto const& child: next->left_children)
				{ nodes.push(&child); }

				for(auto const& child: next->right_children)
				{ nodes.push(&child); }

				nodes.push(nullptr);
			}
			else
			{ f(args...); }

			nodes.pop();
		}
	}

	ridge_tree generate(size_t max_depth);
}

#endif