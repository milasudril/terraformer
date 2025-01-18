//@	{"target":{"name":"elevation_color_map.test"}}

#include "./elevation_color_map.hpp"

#include <testfwk/testfwk.hpp>

static_assert(
	terraformer::get_elevation_color_lut()(0.5f) ==
		normalize(
			terraformer::rgba_pixel{0.0f, 1.0f, 0.0f, 1.0f},
			terraformer::perceptual_color_intensity(0.5f + 1.0f/6.0f)
		)
);