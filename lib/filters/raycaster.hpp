#ifndef TERRAFORMER_FILTERS_RAYCASTER_HPP
#define TERRAFORMER_FILTERS_RAYCASTER_HPP

#include "lib/common/span_2d.hpp"
#include "lib/common/spaces.hpp"

#include <optional>

namespace terraformer
{
	inline std::optional<pixel_coordinates> raycast(span_2d<float const> heighmap,
		pixel_coordinates loc,
		float src_altitude,
		geosimd::rotation_angle src_dir)
	{
		auto const x_0 = static_cast<int32_t>(loc.x);
		auto const y_0 = static_cast<int32_t>(loc.y);
		location r{static_cast<float>(x_0), static_cast<float>(y_0), src_altitude};
		direction const dr{cossin(src_dir), geosimd::dimension_tag<2>{}};

		while(inside(heighmap, r[0], r[1]))
		{
			if(r[2] < interp(heighmap, r[0], r[1]))
			{
				return pixel_coordinates{
					static_cast<uint32_t>(r[0] + 0.5f),
					static_cast<uint32_t>(r[1] + 0.5f)
				};
			}
			r += 1.0f*dr;
		}
		return std::nullopt;
	}
}

#endif