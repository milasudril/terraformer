//@	{"target": {"name":"./ridge_tree.o"}}

#include "./ridge_tree.hpp"

#include "lib/common/tree_generator.hpp"

std::optional<terraformer::ridge_tree_node> terraformer::ridge_tree_builder::make_node()
{
	return terraformer::ridge_tree_node{
		.level = 0
	};
}

std::optional<terraformer::ridge_tree_node> terraformer::ridge_tree_builder::make_node(ridge_tree_node const&)
{
	return std::nullopt;
}

void terraformer::ridge_tree_builder::save_node(ridge_tree_node&&){}

terraformer::ridge_tree terraformer::generate_tree()
{
	ridge_tree ret{};
	generate_tree_bfs(ridge_tree_builder{ret});
	return ret;
}