//@	{"dependencies_extra":[{"ref": "./intensity_map_presentation_filters.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_INTENSITY_MAP_PRESENTATION_FILTERS_HPP
#define TERRAFORMER_INTENSITY_MAP_PRESENTATION_FILTERS_HPP

#include "lib/pixel_store/image.hpp"

namespace terraformer
{
	struct image_resize_description
	{
		uint32_t output_width;
		uint32_t output_height;
	};

	grayscale_image resize(grayscale_image const& src, image_resize_description const& resize_params);
}

#endif