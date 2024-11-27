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

	struct flat_rectangle
	{
		span_2d_extents domain_size;
		uint32_t origin_x;
		uint32_t origin_y;
		uint32_t width;
		uint32_t height;
		uint32_t border_thickness;
		rgba_pixel border_color;
		rgba_pixel fill_color;
	};

	image generate(flat_rectangle const& params);

	image convert_mask(span_2d<uint8_t const> input);
}

#endif
