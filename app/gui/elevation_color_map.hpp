#ifndef TERRAFORMER_ELEVATION_COLOR_HPP
#define TERRAFORMER_ELEVATION_COLOR_HPP

#include "lib/math_utils/interp.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"
#include "lib/pixel_store/intensity.hpp"
#include "ui/value_maps/asinh_value_map.hpp"

namespace terraformer
{
	constexpr std::array<float, 4> elevation_levels{0.0f, 0.5f, 11.0f/12.0f, 12.0f/12.0f};
	constexpr std::array<terraformer::rgba_pixel, 4> elevation_colors{
		rgba_pixel{0.0f, 0.0f, 0.0f, 1.0f}
		+ normalize(rgba_pixel{0.0f, 0.0f, 1.0f, 0.0f}, perceptual_color_intensity(0.5f)),
		rgba_pixel{0.0f, 0.0f, 0.0f, 1.0f}
		+ normalize(rgba_pixel{0.0f, 1.0f, 0.0f, 0.0f}, perceptual_color_intensity(0.5f + 1.0f/6.0f)),
		rgba_pixel{0.0f, 0.0f, 0.0f, 1.0f}
		+ normalize(rgba_pixel{1.0f, 0.0f, 0.0f, 0.0f}, perceptual_color_intensity(0.5f + 2.0f/6.0f)),
		rgba_pixel{0.0f, 0.0f, 0.0f, 1.0f}
		+ normalize(rgba_pixel{0.3333f, 0.3333f, 0.3333f, 0.0f}, perceptual_color_intensity(0.5f + 3.0f/6.0f))
	};

	constexpr auto get_elevation_color_lut()
	{
		return terraformer::linear_interpolation_table{
			terraformer::multi_span<float const, terraformer::rgba_pixel const>{
				std::data(elevation_levels),
				std::data(elevation_colors),
				std::size(elevation_levels)
			}
		};
	}

	constexpr ui::value_maps::asinh_value_map global_elevation_map{
		266.3185546307779f,
		0.7086205026374324f*6.0f
	};
}

#endif