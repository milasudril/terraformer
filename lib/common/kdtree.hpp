//@	{"dependencies_extra":[{"ref":"./kdtree.o","rel":"implementation"}]}

#ifndef TERRAFORMER_KDTREE_HPP
#define TERRAFORMER_KDTREE_HPP

#include "./spaces.hpp"

#include <memory>
#include <vector>

namespace terraformer
{
	struct point_and_distance
	{
		location loc;
		float distance;
	};

	struct kdtree_2d_node
	{
		explicit kdtree_2d_node(std::span<location> locs, size_t level);

		location loc;
		std::unique_ptr<kdtree_2d_node> left;
		std::unique_ptr<kdtree_2d_node> right;

		template<class Callable>
		void visit_nodes(Callable const& cb) const
		{
			cb(loc);
			if(left != nullptr)
			{ left->visit_nodes(cb); }
			if(right != nullptr)
			{ right->visit_nodes(cb); }
		}

		point_and_distance closest_point(location target, size_t level) const;
	};


	class kdtree_2d
	{
	public:
		explicit kdtree_2d(std::vector<location>&& locs):
			m_root{locs, 0}
		{}

		template<class Callable>
		void visit_nodes(Callable const& cb) const
		{ m_root.visit_nodes(cb); }

		point_and_distance closest_point(location loc) const
		{ return m_root.closest_point(loc, 0); }

	private:
		kdtree_2d_node m_root;
	};
}

#endif