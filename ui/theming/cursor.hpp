#ifndef TERRAFORMER_UI_THEMING_CURSOR_HPP
#define TERRAFORMER_UI_THEMING_CURSOR_HPP

#include "lib/pixel_store/rgba_pixel.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/pixel_store/image.hpp"
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
	auto create_cursor(CursorFactory&& factory, cursor_view const& cursor, rgba_pixel color)
	{
		using pixel_type = std::remove_cvref_t<CursorFactory>::cursor_pixel_type;
		basic_image<pixel_type> output{cursor.pixels.width(), cursor.pixels.height()};
		std::array<rgba_pixel, 4> color_map{
			rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
			rgba_pixel{0.0f, 0.0f, 0.0f, 1.0f},
			color,
			rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f}
		};
		for(uint32_t y = 0; y != output.height(); ++y)
		{
			for(uint32_t x = 0; x != output.width(); ++x)
			{
				auto const color_index = cursor.pixels(x, y);
				output(x, y) = factory.make_cursor_pixel(color_map[color_index]);
			}
		}

		return factory.create_cursor(output.pixels(), cursor.x_hot, cursor.y_hot);
	}
}

#endif