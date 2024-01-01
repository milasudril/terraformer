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

#if 0
terraformer::ridge_tree_branch_sequence::ridge_tree_branch_sequence(ridge_tree_branch const& parent)
{

}


#ifndef TERRAFORMER_RIDGE_TREE_BRANCH_SEQUENCE_HPP
#define TERRAFORMER_RIDGE_TREE_BRANCH_SEQUENCE_HPP

#include "./ridge_tree_branch.hpp"

namespace terraformer
{
	class ridge_tree_branch_sequence
	{
		public:
			explicit ridge_tree_branch_sequence(ridge_tree_branch const& parent);
			
			struct per_side_info
			{
				std::vector<ridge_tree_branch> branches;

				// TODO: Only location is needed for this one
				std::vector<array_tuple<location, float>> delimiters;
				bool first_is_branch;
			};
			
			auto const& left() const
			{ return m_left; }
			
			auto const& right() const
			{ return m_right; }

		private:
			per_side_info m_left;
			per_side_info m_right;
	};

}

#endif
#endif
