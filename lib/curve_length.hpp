#ifndef TERRAFORMER_CURVE_LENGTH_HPP
#define TERRAFORMER_CURVE_LENGTH_HPP

#include "lib/common/spaces.hpp"

#include <span>
#include <cassert>

namespace terraformer
{
	inline auto curve_length(std::span<location const> locs)
	{
		auto sum = 0.0f;
		if(std::size(locs) == 0)
		{ return sum; }

		for(size_t k = 1; k != std::size(locs); ++k)
		{ sum += distance(locs[k], locs[k - 1]); }

		return sum;
	}
}

#endif