//@	{"target":{"name":"./kdtree.o"}}

#include "./kdtree.hpp"

#include <algorithm>

terraformer::kdtree_2d_node::kdtree_2d_node(std::span<location> locs, size_t level)
{
	std::ranges::nth_element(locs, locs.begin() + std::size(locs)/2, [dim = level % 2](auto a, auto b){
		return a[dim] < b[dim];
	});
	loc = locs[std::size(locs)/2];
	auto const left_span = locs.subspan(0, std::size(locs)/2);
	auto const right_span = locs.subspan(std::size(locs)/2 + 1);
	if(std::size(left_span) != 0)
	{ left = std::make_unique<kdtree_2d_node>(left_span, level + 1); }

	if(std::size(right_span) != 0)
	{ right = std::make_unique<kdtree_2d_node>(right_span, level + 1); }
}


terraformer::point_and_distance terraformer::kdtree_2d_node::closest_point(location target, size_t level) const
{
	point_and_distance current_best{
		.loc = loc,
		.distance = distance_xy(loc, target)
	};

	auto const dim = level % 2;
	auto const recursion = target[dim] < loc[dim]?
		 std::pair{left.get(), right.get()}:
		 std::pair{right.get(), left.get()};

	{
		auto const new_best = recursion.first != nullptr?
			recursion.first->closest_point(target, level + 1):
			current_best;

		if(new_best.distance < current_best.distance)
		{ current_best = new_best; }
	}

	if(std::abs(target[dim] - loc[dim]) < current_best.distance)
	{
		auto const new_best = recursion.second != nullptr?
			recursion.second->closest_point(target, level + 1):
			current_best;
		if(new_best.distance < current_best.distance)
		{ current_best = new_best; }
	}

	return current_best;
}