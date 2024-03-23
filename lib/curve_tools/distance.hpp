#ifndef TERRAFORMER_CURVE_DISTANCE_HPP
#define TERRAFORMER_CURVE_DISTANCE_HPP

#include "lib/array_classes/span.hpp"

namespace terraformer
{
	struct curve_distance_result
	{
		location loc{0.0f, 0.0f, 0.0f};
		float distance{-1.0f};
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

	inline auto curve_closest_point_xy_2(span<location const> curve, location loc)
	{
		if(curve.empty())
		{ return std::array<curve_distance_result, 2>{}; }

		auto const d_first = distance_xy(curve.front(), loc);

		std::array<curve_distance_result, 2> ret{
			curve_distance_result{
				.loc = curve.front(),
				.distance = d_first
			},
			curve_distance_result{
				.loc = curve.front(),
				.distance = d_first
			},
		};

		for(auto k = curve.first_element_index() + 1; k != std::size(curve); ++k)
		{
			auto const d = distance_xy(curve[k], loc);
			if(d < ret[1].distance)
			{
				ret[0] = ret[1];
				ret[1].loc = curve[k];
				ret[1].distance = d;
			}
		}

		return ret;
	}
}

#endif