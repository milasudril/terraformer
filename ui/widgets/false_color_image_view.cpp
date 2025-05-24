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