#ifndef TERRAFORMER_CURVE_LENGTH_HPP
#define TERRAFORMER_CURVE_LENGTH_HPP

#include "./line_segment.hpp"

namespace terraformer
{
	inline auto curve_length(std::span<location const> locs)
	{
		return fold_over_line_segments(
			locs,
			[](auto lineseg, auto ... vals){
				return (vals + ... + length(lineseg));
			}
		);
	}

	inline auto curve_length_xy(std::span<location const> locs)
	{
		return fold_over_line_segments(
			locs,
			[](auto lineseg, auto ... vals){
				return (vals + ... + length_xy(lineseg));
			}
		);
	}
}

#endif