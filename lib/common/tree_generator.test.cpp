//@	{"target":{"name":"tree_generator.test"}}

#include "./tree_generator.hpp"

#include <testfwk/testfwk.hpp>

namespace
{
	struct tree_node
	{
		size_t parent_id;
		size_t id;
		size_t level;
		size_t index;
	};

	struct tree_builder
	{
		std::reference_wrapper<std::vector<tree_node>> nodes;
		size_t current_parent_id{static_cast<size_t>(-1)};
		size_t current_id{0};
		size_t current_level{0};
		size_t current_index{0};
		size_t child_count{0};

		std::optional<tree_node> make_node()
		{
			return tree_node{
				.parent_id = static_cast<size_t>(-1),
				.id = current_id,
				.level = 0,
				.index = 0
			};
		}

		std::optional<tree_node> make_node(tree_node const& parent)
		{
			if(parent.level + 1 != current_level)
			{
				current_level = parent.level + 1;
				current_index = 0;
			}
			else
			{ ++current_index; }

			if(parent.id != current_parent_id)
			{
				current_parent_id = parent.id;
				child_count = 0;
			}
			else
			{
				++child_count;
			}

			if(current_level < 4 && child_count < 3)
			{
				++current_id;
				return tree_node{
					.parent_id = parent.id,
					.id = current_id,
					.level = parent.level + 1,
					.index =  current_index
				};
			}
			return std::nullopt;
		}

		void save_node(tree_node&& node)
		{
			nodes.get().push_back(std::move(node));
		}
	};
}

TESTCASE(terraformer_tree_generator_generate_tree)
{
	std::vector<tree_node> nodes;
	terraformer::generate_tree_bfs(tree_builder{std::ref(nodes)});

	for(size_t k = 0; k != std::size(nodes); ++k)
	{
		printf("parent_id = %zu, id = %zu, level = %zu, index = %zu\n",
					 nodes[k].parent_id, nodes[k].id, nodes[k].level, nodes[k].index);
	}
}