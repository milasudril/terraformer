//@	{"dependencies_extra": [{"ref": "./ridge_tree_branch_sequence.o","rel":"implementation"}]}

#ifndef TERRAFORMER_RIDGE_TREE_BRANCH_SEQUENCE_HPP
#define TERRAFORMER_RIDGE_TREE_BRANCH_SEQUENCE_HPP

#include "./ridge_tree_branch.hpp"
#include "./ridge_curve.hpp"
#include "./boundary_sampling_policies.hpp"
#include "./curve_length.hpp"

#include "lib/common/span_2d.hpp"

namespace terraformer
{
	template<class BranchStopCondition>
	std::vector<location> generate_branch_base_curve(
		location loc,
		direction start_dir,
		span_2d<float const> potential,
		float pixel_size,
		BranchStopCondition&& stop)
	{
		std::vector<location> base_curve;
		if(stop(loc) || !inside(potential, loc[0]/pixel_size, loc[1]/pixel_size))
		{ return base_curve; }

		base_curve.push_back(loc);

		loc += pixel_size*start_dir;

		while(!stop(loc) && inside(potential, loc[0]/pixel_size, loc[1]/pixel_size))
		{
			base_curve.push_back(loc);
			auto const g = direction{
				grad(
					potential,
					loc[0]/pixel_size,
					loc[1]/pixel_size,
					1.0f,
					clamp_at_boundary{}
				)
			};

			loc -= pixel_size*g;
		}
		return base_curve;
	}
	
	
	std::vector<array_tuple<location, float>>
	generate_delimiters(
		array_tuple<location, direction> const& delimiter_points,
		span_2d<float const> potential,
		float pixel_size,
		ridge_curve_description curve_desc,
		random_generator& rng,
		std::vector<array_tuple<location, float>>&& existing_delimiters =  std::vector<array_tuple<location, float>>{});
	
	
	class ridge_tree_branch_sequence
	{
		public:
			explicit ridge_tree_branch_sequence(
				ridge_tree_branch const& parent,
				span_2d<float const> potential,
				float pixel_size,
				ridge_curve_description curve_desc,
				random_generator& rng
			);
			
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
