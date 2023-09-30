//@	{"dependencies_extra":[{"ref": "./intensity_map_presentation_filters.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_INTENSITY_MAP_PRESENTATION_FILTERS_HPP
#define TERRAFORMER_INTENSITY_MAP_PRESENTATION_FILTERS_HPP

#include "lib/pixel_store/image.hpp"

#include <ranges>
#include <optional>

namespace terraformer
{
	struct image_resize_description
	{
		uint32_t output_width;
		uint32_t output_height;
	};

	grayscale_image resize(grayscale_image const& src, image_resize_description const& resize_params);

	struct posterization_description
	{
		uint32_t levels;
	};

	std::pair<grayscale_image, std::ranges::minmax_result<float>> posterize(grayscale_image const& src, posterization_description const& params);

	std::pair<grayscale_image, std::ranges::minmax_result<float>>
	generate_level_curves(grayscale_image const& src,
	posterization_description const& params);

	image apply_colormap(grayscale_image const& src,
		std::span<rgba_pixel const> colors,
		std::optional<std::ranges::minmax_result<float>> range = std::nullopt);
}

#endif