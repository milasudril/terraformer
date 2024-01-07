#ifndef TERRAFORMER_CURVE_PAIRWISE_MIDPOINTS_HPP
#define TERRAFORMER_CURVE_PAIRWISE_MIDPOINTS_HPP

#include "lib/common/spaces.hpp"

#include <span>
#include <algorithm>

namespace terraformer
{
	inline std::vector<location> pairwise_midpoints(
		std::span<location const> a,
		std::span<location const> b)
	{
		std::vector<location> ret(std::min(std::size(a), std::size(b)));

		for(size_t k = 0; k != std::size(ret); ++k)
		{ ret[k] = midpoint(a[k], b[k]); }

		return ret;
	}
}

#endif
