#ifndef TERRAFORMER_CURVE_LENGTH_HPP
#define TERRAFORMER_CURVE_LENGTH_HPP

#include "./line_segment.hpp"
#include "lib/array_classes/span.hpp"
#include "lib/array_classes/single_array.hpp"

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

	inline auto curve_running_length_xy(span<location const> locs)
	{
		if(locs.empty())
		{ return single_array<float>{}; }

		single_array ret{array_size<float>{std::size(locs).get()}};
		auto sum = 0.0f;
		ret.front() = sum;

		for(auto k = locs.first_element_index() + 1; k != std::size(locs); ++k)
		{
			auto const d = distance_xy(locs[k], locs[k - 1]);
			sum += d;
			array_index<float> const output_index{k.get()};
			ret[output_index] = sum;
		}

		return ret;
	}
}

#endif