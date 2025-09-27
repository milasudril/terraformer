//@	{"target": {"name":"./ridge_tree.o"}}

#include "./ridge_tree.hpp"
#include "./ridge_tree_branch.hpp"

#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/utils.hpp"
#include "lib/curve_tools/length.hpp"
#include "lib/math_utils/interp.hpp"
#include "lib/math_utils/cubic_spline.hpp"

terraformer::ridge_tree_trunk terraformer::generate_trunk(
	ridge_tree_trunk_curve_descriptor const& params,
	ridge_tree_branch_displacement_description const& horz_displacement_profile,
	random_generator& rng
)
{
	auto const trunk_e2e_distance = distance(params.begin.y, params.end.y);
	auto const trunk_pixel_size = get_min_pixel_size(horz_displacement_profile);

	auto const trunk_pixel_count = static_cast<size_t>(trunk_e2e_distance/trunk_pixel_size);

	auto const trunk_offsets = generate(
		horz_displacement_profile,
		rng,
		array_size<float>{trunk_pixel_count},
		trunk_pixel_size,
		1024.0f
	);

	auto const trunk_base_curve = make_point_array
		(params.begin, params.end, trunk_pixel_count);
		auto curve = displace_xy(
			trunk_base_curve,
			displacement_profile{
				.offsets = trunk_offsets,
				.sample_period = trunk_pixel_size
			}
		);

		auto integrated_curve_length = curve_running_length_xy(std::as_const(curve).points());

		ridge_tree_branch_sequence root;
		root.push_back(
			std::move(curve),
			displaced_curve::index_type{},
			single_array<displaced_curve::index_type>{},
			std::move(integrated_curve_length)
		);

	return ridge_tree_trunk{
		.level = 0,
		.branches = std::move(root),
		.parent = ridge_tree_trunk::no_parent,
		.parent_curve_index = array_index<displaced_curve>{0},
		.side = ridge_tree_trunk::side::left,
		.elevation_data = ridge_tree_branch_elevation_data{}
	};
}
