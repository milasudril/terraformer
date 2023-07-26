#ifndef TERRAFORMER_FILTERS_FILL_FROM_POINTS_HPP
#define TERRAFORMER_FILTERS_FILL_FROM_POINTS_HPP

#include "lib/common/span_2d.hpp"
#include "lib/common/spaces.hpp"

#include <span>

namespace terraformer
{
	template<class Callable>
	void fill_from_points(span_2d<float> output,
		std::span<location const> points,
		Callable&& point_to_intensity)
	{
		for(uint32_t y = 0; y != output.height(); ++y)
		{
			for(uint32_t x =0; x != output.width(); ++x)
			{
				auto const P = location{static_cast<float>(x), static_cast<float>(y), 0.0f};
 				output(x, y) = point_to_intensity(P, points);
			}
		}
	}
}

#endif