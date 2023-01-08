#ifndef TERRAFORMER_LIB_REPULSION_HPP
#define TERRAFORMER_LIB_REPULSION_HPP

#include "./spaces.hpp"

#include <span>

namespace terraformer
{
	inline displacement repulsion_between(location r0, std::span<location const> data_points)
	{
		if(std::size(data_points) == 0) [[unlikely]]
		{ return displacement{}; }

		displacement ret;
		for(size_t k = 1; k != std::size(data_points); ++k)
		{
			auto const ds = distance(data_points[k], data_points[k - 1]);
			auto const s_m = midpoint(data_points[k], data_points[k - 1]);
			auto const r = r0 - s_m;
			ret += (ds*r)/(norm(r)*norm_squared(r));
		}

		return ret;
	}
}

#endif