//@	{"dependencies_extra":[{"ref":"./ridge_tree.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_HPP
#define TERRAFORMER_RIDGE_TREE_HPP

#include "lib/common/array_tuple.hpp"
#include "lib/common/spaces.hpp"

namespace terraformer
{
	enum class ridge_tree_branch_side{left, right};

	constexpr auto ridge_tree_locations = 0;
	constexpr auto ridge_tree_normals = 1;
	constexpr auto ridge_tree_levels = 2;
	constexpr auto ridge_tree_sides = 3;
	using ridge_tree = array_tuple<location, direction, size_t, ridge_tree_branch_side>;

	struct ridge_tree_node
	{
		size_t level;
	};

	class ridge_tree_builder
	{
	public:
		explicit ridge_tree_builder(ridge_tree& output):m_output{output}{}

		std::optional<ridge_tree_node> make_node();

		std::optional<ridge_tree_node> make_node(ridge_tree_node const&);

		void save_node(ridge_tree_node&& node);

	private:
		std::reference_wrapper<ridge_tree> m_output;
	};

	ridge_tree generate_tree();
}

#endif