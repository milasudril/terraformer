//@	{"target":{"name":"ridge_tree_branch.o"}}

#include "./ridge_tree_branch.hpp"
#include "./find_zeros.hpp"

terraformer::ridge_tree_branch::ridge_tree_branch(
	std::span<location const> base_curve,
	displacement_profile dy):
	m_points{displace_xy(base_curve, dy)}
{
	auto const points = m_points.get<0>();
	auto const offsets = m_points.get<1>();

	auto const x_intercepts = terraformer::find_zeros(offsets);
	auto side = x_intercepts.first_value >= 0.0f ? 1.0f : -1.0f;
	m_left_seeds.first_is_branch = x_intercepts.first_value >= 0.0f;
	size_t l = 0;
	if(l != std::size(x_intercepts.zeros) && x_intercepts.zeros[l] == 0)
	{
		++l;
		side = -side;
	}

	float max_offset = 0.0f;
	std::optional<size_t> selected_branch_point;
	std::vector<size_t> branch_at_tmp;
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
				if(side >= 0.0f)
				{ m_left_seeds.branch_points.push_back(tuple{loc_b, normal}); }
				else
				{ m_right_seeds.branch_points.push_back(tuple{loc_b, normal}); }
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
		{ m_left_seeds.branch_points.push_back(tuple{loc_b, normal}); }
		else
		{ m_right_seeds.branch_points.push_back(tuple{loc_b, normal}); }
	}

	std::ranges::reverse(m_right_seeds.branch_points.get<0>());
	std::ranges::reverse(m_right_seeds.branch_points.get<1>());

	m_right_seeds.first_is_branch =
		(
			std::size(m_right_seeds.branch_points) == std::size(m_left_seeds.branch_points)
			&& m_left_seeds.first_is_branch
		) ||
		(
			std::size(m_right_seeds.branch_points) != std::size(m_left_seeds.branch_points)
			&& !m_left_seeds.first_is_branch
		);
}
