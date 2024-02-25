//@	{"target":{"name": "./ridge_tree_branch_seed_sequence.o"}}

#include "./ridge_tree_branch_seed_sequence.hpp"

#include "lib/curve_tools/displace.hpp"
#include "lib/common/find_zeros.hpp"

terraformer::ridge_tree_branch_seed_sequence_pair terraformer::collect_ridge_tree_branch_seeds(
	displaced_curve const& displaced_points
)
{
	auto const points = displaced_points.get<0>();
	auto const offsets = displaced_points.get<1>();

	auto const x_intercepts = terraformer::find_zeros(offsets);
	auto side = x_intercepts.first_value >= 0.0f ? 1.0f : -1.0f;
	size_t l = 0;
	if(l != std::size(x_intercepts.zeros) && x_intercepts.zeros[l] == 0)
	{
		++l;
		side = -side;
	}

	ridge_tree_branch_seed_sequence_pair ret;
	float max_offset = 0.0f;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
	std::optional<displaced_curve::index_type> selected_branch_point;
#pragma GCC diagnostic pop
	for(auto k = displaced_points.first_element_index() + 1;
		k != displaced_points.last_element_index();
		++k
	)
	{
		if(l != std::size(x_intercepts.zeros) && k == displaced_curve::index_type{x_intercepts.zeros[l]})
		{
			if(selected_branch_point.has_value())
			{
				auto const loc_a = points[*selected_branch_point - 1];
				auto const loc_b = points[*selected_branch_point];
				auto const loc_c = points[*selected_branch_point + 1];
				auto const normal = curve_vertex_normal_from_curvature(loc_a, loc_b, loc_c);
				if(side >= 0.0f)
				{ ret.left.push_back(loc_b, normal, *selected_branch_point); }
				else
				{ ret.right.push_back(loc_b, normal, *selected_branch_point); }
			}
			max_offset = 0.0f;
			++l;
			side = -side;
			selected_branch_point.reset();
		}

		auto const y = offsets[k];
		auto const loc_a = points[k - 1];
		auto const loc_b = points[k];
		auto const loc_c = points[k + 1];
		auto const normal
			= curve_vertex_normal_from_projection(loc_a, loc_b, loc_c, displacement{0.0f, 0.0f, -1.0f});
		auto const ab = loc_b - loc_a;
		auto const side_of_curve = inner_product(ab, normal);
		auto const visible = (side*y > 0.0f ? 1.0f : 0.0f)*(side*side_of_curve > 0.0f ? 1.0f : 0.0f);

		if(visible && std::abs(y) > max_offset && side*y > side*offsets[k - 1] && side*y < side*offsets[k + 1])
		{
			max_offset = std::abs(y);
			selected_branch_point = k;
		}
	}

	if(selected_branch_point.has_value())
	{
		auto const loc_a = points[*selected_branch_point - 1];
		auto const loc_b = points[*selected_branch_point];
		auto const loc_c = points[*selected_branch_point + 1];
		auto const normal = curve_vertex_normal_from_curvature(loc_a, loc_b, loc_c);
		if(side >= 0.0f)
		{ ret.left.push_back(loc_b, normal, *selected_branch_point); }
		else
		{ ret.right.push_back(loc_b, normal, *selected_branch_point); }
	}

	// Reverse the order of branch points on the right hand side. This way, all branches will be
	// located to the left of the trunk, which makes it easier to do collision detection in a
	// later step.
	std::ranges::reverse(ret.right.get<0>());
	std::ranges::reverse(ret.right.get<1>());
	std::ranges::reverse(ret.right.get<2>());

	return ret;
}

terraformer::single_array<terraformer::ridge_tree_branch_seed_sequence_pair>
terraformer::collect_ridge_tree_branch_seeds(span<displaced_curve const> points)
{
	single_array<terraformer::ridge_tree_branch_seed_sequence_pair> ret;
	ret.reserve(static_cast<decltype(ret)::size_type>(std::size(points)));
	for(auto k = points.first_element_index(); k != std::size(points); ++k)
	{
		ret.push_back(collect_ridge_tree_branch_seeds(points[k]));
	}
	return ret;
}

terraformer::single_array<terraformer::displaced_curve::index_type>
terraformer::collect_branch_indices(ridge_tree_branch_seed_sequence_pair const& seq_pair)
{
	single_array<displaced_curve::index_type> ret;
	decltype(ret)::size_type const total_size{std::size(seq_pair.left) + std::size(seq_pair.right)};
	ret.reserve(total_size);
	std::ranges::copy(seq_pair.left.get<2>(), std::back_inserter(ret));
	std::ranges::copy(seq_pair.right.get<2>(), std::back_inserter(ret));
	std::ranges::sort(ret);
	return ret;
}