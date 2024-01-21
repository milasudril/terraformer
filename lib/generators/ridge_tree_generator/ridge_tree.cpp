//@	{"target": {"name":"./ridge_tree.o"}}

#include "./ridge_tree.hpp"

std::vector<terraformer::ridge_tree_branch>
terraformer::generate_branches(
	array_tuple<location, direction> const& branch_points,
	span_2d<float const> potential,
	float pixel_size,
	ridge_curve_description curve_desc,
	random_generator& rng,
	float d_max,
	std::vector<ridge_tree_branch>&& existing_branches)
{
	auto const points = branch_points.get<0>();
	auto const normals = branch_points.get<1>();
	for(size_t k = 0; k != std::size(branch_points); ++k)
	{
		auto const base_curve = generate_branch_base_curve(
			points[k],
			normals[k],
			potential,
			pixel_size,
			[d = 0.0f, loc_prev = points[k], d_max](auto loc) mutable {
				auto new_distance = d + distance(loc, loc_prev);
				if(new_distance > d_max)
				{ return true; }
				d = new_distance;
				loc_prev = loc;
				return false;
			}
		);

		if(std::size(base_curve) < 3)
		{
			fprintf(stderr, "Curve %zu is to short\n", k);
			continue;
		}

		auto const base_curve_length = static_cast<size_t>(curve_length(base_curve)/pixel_size) + 1;
		auto const offsets = generate(curve_desc, rng, base_curve_length, pixel_size);

		existing_branches.push_back(
			displace_xy(
				base_curve,
				displacement_profile{
					.offsets = offsets,
					.sample_period = pixel_size,
				}
			)
		);
	}

	return existing_branches;
}

terraformer::ridge_tree terraformer::generate(size_t)
{
	terraformer::ridge_tree ret{};


	return ret;
}

