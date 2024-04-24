#ifndef TERRAFORMER_UI_THEMING_STATIC_CURSOR_HPP
#define TERRAFORMER_UI_THEMING_STATIC_CURSOR_HPP

#include "lib/pixel_store/rgba_pixel.hpp"
#include <array>

namespace terraformer::ui::theming
{
	template<uint32_t Width, uint32_t Height>
	struct static_cursor
	{
		int x_hot;
		int y_hot;
		std::array<int, Width*Height> pixels;
	};

	template<class CursorFactory, uint32_t Width, uint32_t Height>
	auto create_cursor(static_cursor<Width, Height> const& cursor, rgba_pixel color)
	{
		using pixel_type = CursorFactory::pixel_type;
		std::array<pixel_type, Width*Height> pixels{};
		std::array<rgba_pixel, 3> color_map{
			rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
			rgba_pixel{0.0f, 0.0f, 0.0f, 1.0f},
			color,
			rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f}
		};
		for(size_t k = 0; k != std::size(pixels); ++k)
		{ pixels[k] = CursorFactory::make_cursor_pixel(color_map[cursor.pixels[k]]); }
		return CursorFactory::template create_cursor<Width, Height>(std::as_bytes(pixels), cursor.x_hot, cursor.y_hot);
	}
}

#endif