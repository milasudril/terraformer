//@	{"target":{"name": "./ridge_tree_branch_seed_sequence.o"}}

#include "./ridge_tree_branch_seed_sequence.hpp"

#include "lib/curve_tools/displace.hpp"
#include "lib/common/find_zeros.hpp"

terraformer::ridge_tree_branch_seed_sequence terraformer::collect_ridge_tree_branch_seeds(
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

	ridge_tree_branch_seed_sequence ret;
	float max_offset = 0.0f;
	std::optional<size_t> selected_branch_point;
	for(size_t k = 1; k != std::size(offsets) - 1;++k)
	{
		if(l != std::size(x_intercepts.zeros) && k == x_intercepts.zeros[l])
		{
			if(selected_branch_point.has_value())
			{
				auto const loc_a = points[*selected_branch_point - 1];
				auto const loc_b = points[*selected_branch_point];
				auto const loc_c = points[*selected_branch_point + 1];
				auto const normal = curve_vertex_normal_from_curvature(loc_a, loc_b, loc_c);
				ret.branch_index.push_back(*selected_branch_point);
				if(side >= 0.0f)
				{ ret.left.push_back(tuple{loc_b, normal}); }
				else
				{ ret.right.push_back(tuple{loc_b, normal}); }
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
		ret.branch_index.push_back(*selected_branch_point);
		if(side >= 0.0f)
		{ ret.left.push_back(tuple{loc_b, normal}); }
		else
		{ ret.right.push_back(tuple{loc_b, normal}); }
	}

	std::ranges::reverse(ret.right.get<0>());
	std::ranges::reverse(ret.right.get<1>());

	return ret;
}

std::vector<terraformer::ridge_tree_branch_seed_sequence> terraformer::collect_ridge_tree_branch_seeds(
	std::span<displaced_curve const> points
)
{
	std::vector<terraformer::ridge_tree_branch_seed_sequence> ret;
	ret.reserve(std::size(points));
	for(size_t k = 0; k != std::size(points); ++k)
	{
		ret.push_back(collect_ridge_tree_branch_seeds(points[k]));
	}
	return ret;
}