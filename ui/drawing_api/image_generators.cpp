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