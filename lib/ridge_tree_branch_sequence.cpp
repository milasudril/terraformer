//@	{"target": {"name": "ridge_tree_branch_sequence.o"}}

#include "./ridge_tree_branch_sequence.hpp"

std::vector<terraformer::array_tuple<terraformer::location, float>>
terraformer::generate_delimiters(
	array_tuple<location, direction> const& delimiter_points,
	span_2d<float const> potential,
	float pixel_size,
	ridge_curve_description curve_desc,
	random_generator& rng,
	std::vector<array_tuple<location, float>>&& existing_delimiters)
{
	auto const points = delimiter_points.get<0>();
	auto const normals = delimiter_points.get<1>();
	for(size_t k = 0; k != std::size(delimiter_points); ++k)
	{
		auto const base_curve = generate_branch_base_curve(
			points[k],
			normals[k],
			potential,
			pixel_size,
			[](auto...){return false;}
		);

		auto const base_curve_length = static_cast<size_t>(curve_length(base_curve)/pixel_size) + 1;
		auto const offsets = generate(curve_desc, rng, base_curve_length, pixel_size);

		existing_delimiters.push_back(
			displace_xy(
				base_curve,
				displacement_profile{
					.offsets = offsets,
					.sample_period = pixel_size,
				}
			)
		);
	}

	return existing_delimiters;
}

