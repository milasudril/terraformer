#ifndef TERRAFORMER_TREE_GENERATOR_HPP
#define TERRAFORMER_TREE_GENERATOR_HPP

#include <concepts>
#include <optional>
#include <utility>
#include <queue>

#include <cstdio>

namespace terraformer
{
	template<class TreeBuilder, class NodeType>
	concept tree_builder = requires(TreeBuilder builder, NodeType x)
	{
		{ builder.make_node() } -> std::same_as<std::optional<NodeType>>;
		{ builder.make_node(std::as_const(x)) } -> std::same_as<std::optional<NodeType>>;
		{ builder.save_node(std::move(x)) } -> std::same_as<void>;
	};

	namespace tree_generator_detal
	{
		template<class TreeBuilder>
		using node_type = decltype(std::declval<TreeBuilder>().make_node())::value_type;
	}

	template<class TreeBuilder, class NodeType = tree_generator_detal::node_type<TreeBuilder>>
	requires tree_builder<TreeBuilder, NodeType>
	void generate_tree_bfs(TreeBuilder&& builder)
	{
		std::queue<std::optional<NodeType>> nodes;
		nodes.push(builder.make_node());

		while(!nodes.empty())
		{
			auto next = std::move(nodes.front());
			if(next.has_value())
			{
				while(true)
				{
					nodes.push(builder.make_node(*next));
					if(!nodes.back().has_value())
					{ break; }
				}
				builder.save_node(std::move(*next));
			}
			nodes.pop();
		}
	}
}

#endif