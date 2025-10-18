//@	{"target":{"name": "./ridge_tree_branch_seed_sequence.o"}}

#include "./ridge_tree_branch_seed_sequence.hpp"

#include "lib/common/spaces.hpp"
#include "lib/common/value_map.hpp"
#include "lib/curve_tools/displace.hpp"
#include "lib/common/find_zeros.hpp"
#include "lib/descriptor_io/descriptor_editor_ref.hpp"
#include "lib/value_maps/affine_value_map.hpp"
#include <geosimd/abstract_spaces.hpp>
#include <geosimd/angle.hpp>
#include <numbers>

terraformer::ridge_tree_branch_seed_sequence_pair terraformer::collect_ridge_tree_branch_seeds(
	displaced_curve const& displaced_points,
	ridge_tree_branch_seed_collection_descriptor const& params
)
{
	if(std::size(displaced_points).get() < 3)
	{ return ridge_tree_branch_seed_sequence_pair{}; }

	auto const points = displaced_points.get<0>();
	auto const offsets = displaced_points.get<1>();
	ridge_tree_branch_seed_sequence_pair ret;
	auto const indices = displaced_points.element_indices();

	{
		auto const start_branches = params.start_branches;
		auto const dtheta = start_branches.spread_angle/
			static_cast<float>(start_branches.branch_count + 1);
		auto const last_tangent = points.front() - points[indices.front() + 2];
		geosimd::rotation_angle const ref_angle{};
		auto const theta_0 = ref_angle + (dtheta - 0.5f*start_branches.spread_angle);
		for(size_t k = 0; k != start_branches.branch_count; ++k)
		{
			auto const theta = theta_0 + static_cast<double>(k)*dtheta;
			direction const v{last_tangent.apply(geosimd::rotation<geom_space>{theta, geosimd::dimension_tag<2>{}})};
			if(theta - ref_angle > geosimd::turn_angle{geosimd::turns{0.5}})
			{ ret.right.push_back(points[indices.front()], v, indices.front(), 0.0f); }
			else
			{ ret.left.push_back(points[indices.front()], v, indices.front(), 0.0f); }
		}
	}

	auto const x_intercepts = terraformer::find_zeros(offsets, 1.0f/128.0f);
	auto side = x_intercepts.first_value >= 0.0f ? 1.0f : -1.0f;
	size_t l = 0;
	if(l != std::size(x_intercepts.zeros) && x_intercepts.zeros[l] == 0)
	{
		++l;
		side = -side;
	}

	float max_offset = 0.0f;
	std::optional<displaced_curve::index_type> selected_branch_point;
	for(auto k : index_range{indices.front() + 1, indices.back() - 1})
	{
		if(l != std::size(x_intercepts.zeros) && k == displaced_curve::index_type{x_intercepts.zeros[l]})
		{
			if(selected_branch_point.has_value())
			{
				auto const loc_a = points[*selected_branch_point - 1];
				auto const loc_b = points[*selected_branch_point];
				auto const loc_c = points[*selected_branch_point + 1];
				auto const normal = curve_vertex_normal_from_curvature(loc_a, loc_b, loc_c);
				if(normal.has_value())
				{
					if(side >= 0.0f)
					{ ret.left.push_back(loc_b, *normal, *selected_branch_point, 0.0f); }
					else
					{ ret.right.push_back(loc_b, *normal, *selected_branch_point, 0.0f); }
				}
			}
			max_offset = 0.0f;
			++l;
			side = -side;
			selected_branch_point.reset();
		}

		auto const y = offsets[k];
		if(std::abs(y) > max_offset && side*y > side*offsets[k - 1] && side*y > side*offsets[k + 1])
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
		if(normal.has_value())
		{
			if(side >= 0.0f)
			{ ret.left.push_back(loc_b, *normal, *selected_branch_point, 0.0f); }
			else
			{ ret.right.push_back(loc_b, *normal, *selected_branch_point, 0.0f); }
		}
	}

	{
		auto const end_brancehs = params.end_brancehs;
		auto const dtheta = end_brancehs.spread_angle/
			static_cast<float>(end_brancehs.branch_count + 1);
		auto const last_tangent = points[indices.back()]  - points[indices.back() - 2];
		geosimd::rotation_angle const ref_angle{};
		auto const theta_0 = ref_angle + (dtheta - 0.5f*end_brancehs.spread_angle);
		for(size_t k = 0; k != end_brancehs.branch_count; ++k)
		{
			auto const theta = theta_0 + static_cast<double>(k)*dtheta;
			direction const v{last_tangent.apply(geosimd::rotation<geom_space>{theta, geosimd::dimension_tag<2>{}})};
			if(theta - ref_angle > geosimd::turn_angle{geosimd::turns{0.5}})
			{ ret.left.push_back(points[indices.back()], v, indices.back(), 0.0f); }
			else
			{ ret.right.push_back(points[indices.back()], v, indices.back(), 0.0f); }
		}
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
terraformer::collect_ridge_tree_branch_seeds(
	span<displaced_curve const> curves,
	ridge_tree_branch_seed_collection_descriptor const& params
)
{
	single_array<terraformer::ridge_tree_branch_seed_sequence_pair> ret;
	ret.reserve(static_cast<decltype(ret)::size_type>(std::size(curves)));
	for(auto const& curve : curves)
	{ ret.push_back(collect_ridge_tree_branch_seeds(curve, params)); }
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

void terraformer::ridge_tree_brach_seed_sequence_boundary_point_descriptor::bind(
	descriptor_editor_ref& editor
)
{
	editor.create_float_input(
		u8"Branch count",
		descriptor_editor_ref::assigner<float>{branch_count},
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::affine_int_value_map{0, 4}},
			.textbox_placeholder_string = u8"1",
			.visual_angle_range = std::nullopt
		}
	);

	editor.create_float_input(
		u8"Spread angle",
		descriptor_editor_ref::assigner<float>{spread_angle},
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::affine_value_map{0.0f, 1.0f}},
			.textbox_placeholder_string = u8"0.123456789",
			.visual_angle_range = std::nullopt
		}
	);
}
