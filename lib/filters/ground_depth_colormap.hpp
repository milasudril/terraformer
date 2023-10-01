#ifndef TERRAFORMER_GROUND_DEPTH_COLORMAP_HPP
#define TERRAFORMER_GROUND_DEPTH_COLORMAP_HPP

#include "lib/pixel_store/rgba_pixel.hpp"

namespace terraformer
{
	inline constexpr std::array<rgba_pixel, 2> ground_depth_colormap{
		rgba_pixel{2.25206137e-01f, 1.44849226e-01f, 1.29944652e-01f},
		rgba_pixel{4.62709963e-01f, 7.51452327e-01f, 7.85837770e-01f}
	};
}
#endif