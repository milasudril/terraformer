//@	{"target": {"name":"contour_plot.o", "rel":"implementation"}}

#include "./contour_plot.hpp"
#include "ui/drawing_api/image_generators.hpp"

terraformer::image
terraformer::ui::widgets::contour_plot::apply_filter(span_2d<float const> input_image) const
{
	auto const w = input_image.width();
	auto const h = input_image.height();

	grayscale_image img_posterized{w, h};
	auto const dz = m_dz;

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{ img_posterized(x, y) = std::round(input_image(x, y)/dz); }
	}

	image output_image{w, h};
	assert(h > 2);
	assert(w > 2);

	for(uint32_t y = 1; y != h - 1; ++y)
	{
		for(uint32_t x = 1; x != w - 1; ++x)
		{
			auto const ddx = img_posterized(x + 1, y) - img_posterized(x - 1, y);
			auto const ddy = img_posterized(x, y + 1) - img_posterized(x, y - 1);

			// NOTE: From the quantization above, the maximum gradient size is known to be sqrt(2)
			auto const val = std::sqrt(ddx*ddx + ddy*ddy)/std::numbers::sqrt2_v<float>;
			output_image(x, y) = val*rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f};
		}
	}

	return output_image;
}