#ifndef TERRAFORMER_CURVE_DISTANCE_HPP
#define TERRAFORMER_CURVE_DISTANCE_HPP

#include "lib/array_classes/span.hpp"

namespace terraformer
{
	struct curve_distance_result
	{
		location loc;
		float distance;
	};

	inline auto curve_closest_point_xy(span<location const> curve, location loc)
	{
		if(curve.empty())
		{
			return curve_distance_result{
				.loc = loc,
				.distance = -1.0f
			};
		}

		curve_distance_result ret{
			.loc = curve.front(),
			.distance = distance_xy(curve.front(), loc)
		};

		for(auto k = curve.first_element_index() + 1; k != std::size(curve); ++k)
		{
			auto const d = distance_xy(curve[k], loc);
			if(d < ret.distance)
			{
				ret.loc = curve[k];
				ret.distance = d;
			}
		}

		return ret;
	}
}

#endif