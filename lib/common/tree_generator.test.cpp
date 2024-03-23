//@	{"target":{"name":"tree_generator.test"}}

#include "./tree_generator.hpp"

#include <testfwk/testfwk.hpp>
#include <algorithm>

namespace
{
	struct tree_node
	{
		size_t parent_id;
		size_t id;
		size_t level;
	};

	struct tree_builder
	{
		std::reference_wrapper<std::vector<tree_node>> nodes;
		size_t current_parent_id{static_cast<size_t>(-1)};
		size_t current_id{0};
		size_t child_count{0};

		std::optional<tree_node> make_node()
		{
			return tree_node{
				.parent_id = static_cast<size_t>(-1),
				.id = current_id,
				.level = 0
			};
		}

		std::optional<tree_node> make_node(tree_node const& parent)
		{
			if(parent.id != current_parent_id)
			{
				current_parent_id = parent.id;
				child_count = 0;
			}
			else
			{
				++child_count;
			}
			auto const current_level = parent.level + 1;
			if(current_level < 4 && child_count < 3)
			{
				++current_id;
				return tree_node{
					.parent_id = parent.id,
					.id = current_id,
					.level = parent.level + 1,
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

TESTCASE(terraformer_tree_generator_generate_tree_bfs)
{
	std::vector<tree_node> nodes;
	terraformer::generate_tree_bfs(tree_builder{std::ref(nodes)});

	auto const count_level_0 =  std::ranges::count_if(nodes, [](auto const& node) {
		return node.level == 0;
	});
	EXPECT_EQ(count_level_0, 1);

	auto const count_level_1 = std::ranges::count_if(nodes, [](auto const& node) {
		return node.level == 1;
	});
	EXPECT_EQ(count_level_1, 3);

	auto const count_level_2 = std::ranges::count_if(nodes, [](auto const& node) {
		return node.level == 2;
	});
	EXPECT_EQ(count_level_2, 9);

	auto const count_level_3 = std::ranges::count_if(nodes, [](auto const& node) {
		return node.level == 3;
	});
	EXPECT_EQ(count_level_3, 27);

	EXPECT_EQ(std::size(nodes), static_cast<size_t>(count_level_0+count_level_1+count_level_2+count_level_3));
}