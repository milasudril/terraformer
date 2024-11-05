//@	{"target":{"name":"image_generators.o"}}

#include "./image_generators.hpp"

terraformer::image terraformer::ui::drawing_api::generate(beveled_rectangle const& params)
{
	auto const w = params.width;
	auto const h = params.height;
	image ret{params.width, params.height};
	auto const border_thickness = params.border_thickness;
	auto const upper_left_color = params.upper_left_color;
	auto const lower_right_color = params.lower_right_color;
	auto const fill_color = params.fill_color;

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const border = (x < h - y) || (y < h/2 && x < w - y)? upper_left_color : lower_right_color;
			ret(x, y) = (x>=border_thickness && x <= w - (border_thickness + 1)) && (y >= border_thickness && y <= h - (border_thickness + 1)) ? fill_color : border;
		}
	}
	return ret;
}

terraformer::image terraformer::ui::drawing_api::generate(flat_rectangle const& params)
{
	auto const w = params.width;
	auto const h = params.height;
	image ret{params.width, params.height};
	auto const border_thickness = params.border_thickness;
	auto const border_color = params.border_color;
	auto const fill_color = params.fill_color;

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			ret(x, y) = (x>=border_thickness && x <= w - (border_thickness + 1)) && (y >= border_thickness && y <= h - (border_thickness + 1)) ? fill_color : border_color;
		}
	}
	return ret;
}

terraformer::image terraformer::ui::drawing_api::convert_mask(
	uint32_t output_width,
	uint32_t output_height,
	span_2d<uint8_t const> input,
	uint32_t margin
)
{
	auto const w = output_width;
	auto const h = output_height;
	auto const w_src = input.width();
	auto const h_src = input.height();
	image ret{w, h};
	for(uint32_t y = margin; y != h - margin; ++y)
	{
		for(uint32_t x = margin; x != w - margin; ++x)
		{
			auto const x_src = x - margin;
			auto const y_src = y - margin;
			auto const mask_val = (x_src < w_src && y_src < h_src)?
				static_cast<float>(input(x_src, y_src))/255.0f:
				0.0f;
			ret(x, y) = rgba_pixel{mask_val, mask_val, mask_val, mask_val};
		}
	}
	return ret;
}