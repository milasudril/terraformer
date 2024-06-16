//@	{"dependencies_extra":[{"ref":"./image_generators.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_DRAWING_API_IMAGE_GENERATORS_HPP
#define TERRAFORMER_UI_DRAWING_API_IMAGE_GENERATORS_HPP

#include "lib/pixel_store/image.hpp"

namespace terraformer::ui::drawing_api
{
	struct beveled_rectangle
	{
		uint32_t width;
		uint32_t height;
		uint32_t border_thickness;
		rgba_pixel upper_left_color;
		rgba_pixel lower_right_color;
		rgba_pixel fill_color;
	};

	image generate(beveled_rectangle const& params);

	// TODO: It should be possible to center input in output image
	image convert_mask(uint32_t output_width, uint32_t output_height, span_2d<uint8_t const> input, uint32_t margin);
}

#endif
