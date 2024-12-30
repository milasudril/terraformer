//@	{"dependencies_extra":[{"ref":"./image_generators.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_DRAWING_API_IMAGE_GENERATORS_HPP
#define TERRAFORMER_UI_DRAWING_API_IMAGE_GENERATORS_HPP

#include "lib/pixel_store/image.hpp"

namespace terraformer::ui::drawing_api
{
	struct beveled_rectangle
	{
		span_2d_extents domain_size;
		uint32_t origin_x;
		uint32_t origin_y;
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

	struct beveled_disc
	{
		span_2d_extents domain_size;
		uint32_t origin_x;
		uint32_t origin_y;
		uint32_t radius;
		uint32_t border_thickness;
		rgba_pixel upper_left_color;
		rgba_pixel lower_right_color;
		rgba_pixel fill_color;
	};

	image generate(beveled_disc const& params);

	struct hand
	{
		span_2d_extents domain_size;
		uint32_t origin_y;
		uint32_t thickness;
		uint32_t length;
		rgba_pixel color;
	};

	inline image generate(hand const& params)
	{
		return generate(flat_rectangle{
			.domain_size = params.domain_size,
			.origin_x = (params.domain_size.width - params.thickness)/2,
			.origin_y = params.origin_y,
			.width = params.thickness,
			.height = params.length,
			.border_thickness = 0,
			.border_color = rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
			.fill_color = params.color
		});
	}

	image convert_mask(span_2d<uint8_t const> input);

	image transpose(span_2d<rgba_pixel const> input);
}

#endif
