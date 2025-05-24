//@	{"target": {"name":"false_color_image_view.o", "rel":"implementation"}}

#include "./false_color_image_view.hpp"

terraformer::image
terraformer::ui::widgets::false_color_image_view::apply_filter(span_2d<float const> input_image) const
{
	auto const w = input_image.width();
	auto const h = input_image.height();

	image output_image{w, h};
	auto const value_map_ptr = m_value_map.get().get_pointer();
	auto const from_value = m_value_map.get().get_vtable().from_value;

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const grayscale_val = std::clamp(from_value(value_map_ptr, input_image(x, y)), 0.0f, 1.0f);
			output_image(x, y) = m_color_map(grayscale_val);
		}
	}
	return output_image;
}

std::optional<terraformer::image>
terraformer::ui::widgets::false_color_image_view::create_foreground(span_2d<float const> input_image) const
{
	if(!m_show_level_curves)
	{ return std::nullopt; }

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