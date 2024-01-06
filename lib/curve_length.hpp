#ifndef TERRAFORMER_CURVE_LENGTH_HPP
#define TERRAFORMER_CURVE_LENGTH_HPP

#include "./line_segment.hpp"

namespace terraformer
{
	inline auto curve_length(std::span<location const> locs)
	{
		return fold_over_line_segments(locs, [](auto lineseg, float prev){
			return prev + length(lineseg);
		},0.0f);
	}
}

#endif