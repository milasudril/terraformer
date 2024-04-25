#ifndef TERRAFORMER_UI_THEMING_CURSOR_HPP
#define TERRAFORMER_UI_THEMING_CURSOR_HPP

#include "lib/pixel_store/rgba_pixel.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/array_classes/single_array.hpp"
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

	struct cursor_view
	{
		int x_hot;
		int y_hot;
		span_2d<int const> pixels;
	};

	template<uint32_t Width, uint32_t Height>
	constexpr auto make_cursor_view(static_cursor<Width, Height> const&& cursor) = delete;

	template<uint32_t Width, uint32_t Height>
	consteval auto make_cursor_view(static_cursor<Width, Height> const& cursor)
	{
		return cursor_view{
			.x_hot = cursor.x_hot,
			.y_hot = cursor.y_hot,
			.pixels = span_2d{Width, Height, std::data(cursor.pixels)}
		};
	}

	template<class CursorFactory>
	auto create_cursor(cursor_view const& cursor, rgba_pixel color)
	{
		using pixel_type = CursorFactory::pixel_type;
		single_array<pixel_type> pixels;
		pixels.resize(cursor.pixels.width() * cursor.pixels.height());
		std::array<rgba_pixel, 3> color_map{
			rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
			rgba_pixel{0.0f, 0.0f, 0.0f, 1.0f},
			color,
			rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f}
		};
		for(auto k = pixels.first_element_index(); k != std::size(pixels); ++k)
		{ pixels[k] = CursorFactory::make_cursor_pixel(color_map[cursor.pixels[k]]); }
		return CursorFactory::create_cursor(std::as_bytes(pixels), cursor.x_hot, cursor.y_hot);
	}
}

#endif