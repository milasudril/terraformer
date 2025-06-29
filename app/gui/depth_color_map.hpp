#ifndef TERRAFORMER_DEPTH_COLOR_HPP
#define TERRAFORMER_DEPTH_COLOR_HPP

#include "lib/math_utils/interp.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"
#include "lib/pixel_store/intensity.hpp"
#include "ui/value_maps/asinh_value_map.hpp"

namespace terraformer
{
	constexpr std::array<float, 2> depth_levels{0.0f, 1.0f};
	constexpr std::array<terraformer::rgba_pixel, 2> depth_colors{
		rgba_pixel{0.0f, 0.0f, 0.0f, 1.0f}
		+ normalize(rgba_pixel{0.5f, 1.0f, 0.0f, 0.0f}, perceptual_color_intensity(0.5f)),
		rgba_pixel{0.0f, 0.0f, 0.0f, 1.0f}
		+ normalize(rgba_pixel{0.0f, 0.5f, 1.0f, 0.0f}, perceptual_color_intensity(0.75f))
	};

	constexpr auto get_depth_color_lut()
	{
		return terraformer::linear_interpolation_table{
			terraformer::multi_span<float const, terraformer::rgba_pixel const>{
				std::data(depth_levels),
				std::data(depth_colors),
				std::size(depth_levels)
			}
		};
	}
}

#endif