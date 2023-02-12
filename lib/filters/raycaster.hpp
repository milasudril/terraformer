#ifndef TERRAFORMER_FILTERS_RAYCASTER_HPP
#define TERRAFORMER_FILTERS_RAYCASTER_HPP

#include "lib/common/span_2d.hpp"
#include "lib/common/spaces.hpp"

namespace terraformer
{
	float source_visible(span_2d<float const> heighmap, int32_t x_0, int32_t y_0, direction src_dir)
	{
		auto const dr = src_dir;
		auto const z_0 = heighmap(x_0, y_0);

		if(std::abs(dr[0]) > std::abs(dr[1]))
		{
			auto const a = dr[1]/dr[0];
			auto const b = dr[2]/dr[0];
			auto const dx = dr[0] >= 0.0f ? 1 : -1;
			for(auto l = x_0;
				l > 0 && l < static_cast<int32_t>(heighmap.width());
				l += dx)
			{
				auto const x = static_cast<float>(l);
				auto const y = a*static_cast<float>(l - x_0) + static_cast<float>(y_0);
				auto const z_ray = b*static_cast<float>(l - x_0) + z_0;
				if(z_ray < interp(heighmap,x, y))
				{ return 0.0f; }
			}
		}
		else
		{
			auto const a = dr[0]/dr[1];
			auto const b = dr[2]/dr[1];
			auto const dy = dr[1] >= 0.0f ? 1 : -1;
			for(auto k = y_0;
				k > 0 && k < static_cast<int32_t>(heighmap.height());
				k += dy)
			{
				auto const y = static_cast<float>(k);
				auto const x = a*static_cast<float>(k - y_0) + static_cast<float>(x_0);
				auto const z_ray = b*static_cast<float>(k - y_0) + z_0;
				if(z_ray < interp(heighmap,x, y))
				{ return 0.0f; }
			}
		}
		return 1.0f;
	}

	void raycast(span_2d<float> lightmap, span_2d<float const> heightmap, direction src_dir)
	{
		for(uint32_t y = 0; y != heightmap.height(); ++y)
		{
			for(uint32_t x = 0; x != heightmap.width(); ++x)
			{
				auto const n = normal(heightmap, x, y);
				lightmap(x, y) = std::max(0.0f, inner_product(n, src_dir))
					* source_visible(heightmap, x, y, src_dir);
			}
		}
	}
}

#endif