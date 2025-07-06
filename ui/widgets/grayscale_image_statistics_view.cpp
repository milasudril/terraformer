//@	{"target":{"name":"grayscale_image_statistics_view.o"}}

#include "./grayscale_image_statistics_view.hpp"
#include "lib/common/utils.hpp"

void terraformer::ui::widgets::grayscale_image_statistics_view::show_image(span_2d<float const> pixels, box_size phys_dim)
{
	// TODO: Should use function that returns u8string
	m_width->value(reinterpret_cast<char8_t const*>(std::to_string(pixels.width()).c_str()));
	m_height->value(reinterpret_cast<char8_t const*>(std::to_string(pixels.height()).c_str()));

	auto const n =pixels.width() * pixels.height();
	auto const minmax = std::minmax_element(pixels.data(), pixels.data() + n);

	auto const min_str = to_string_helper(*minmax.first);
	auto const max_str = to_string_helper(*minmax.second);
	m_min_elev->value(reinterpret_cast<char8_t const*>(min_str.c_str()));
	m_max_elev->value(reinterpret_cast<char8_t const*>(max_str.c_str()));

	auto max_slope = 0.0f;
	auto const w = static_cast<float>(pixels.width());
	auto const h = static_cast<float>(pixels.height());
	auto const dx = phys_dim[0]/w;
	auto const dy = phys_dim[1]/h;
	for(uint32_t y = 1; y != pixels.height() - 1; ++y)
	{
		for(uint32_t x = 1; x != pixels.width() - 1; ++x)
		{
			auto const ddx = (pixels(x + 1, y) - pixels(x - 1, y))/(2.0f*dx);
			auto const ddy = (pixels(x, y + 1) - pixels(x, y - 1))/(2.0f*dy);
			max_slope = std::max(ddx*ddx + ddy*ddy, max_slope);
		}
	}

	auto const max_slope_str = to_string_helper(std::sqrt(max_slope));
	m_max_slope->value(reinterpret_cast<char8_t const*>(max_slope_str.c_str()));
}