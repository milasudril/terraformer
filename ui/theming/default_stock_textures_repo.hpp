#ifndef DEFAULT_STOCK_TEXTURES_REPO_HPP
#define DEFAULT_STOCK_TEXTURES_REPO_HPP

#include "ui/theming/color_scheme.hpp"
#include "lib/common/rng.hpp"
#include "lib/pixel_store/image.hpp"

#include <random>

#include <cstdio>

namespace terraformer::ui::main
{
	template<class DrawingSurface>
	struct default_stock_textures_repo
	{
		using texture_type = DrawingSurface;

		DrawingSurface none;

		DrawingSurface main_panel_background;
		DrawingSurface other_panel_background;
		DrawingSurface input_area_background;
		DrawingSurface command_area_background;
		DrawingSurface output_area_background;

		[[nodiscard]] inline static auto& get_default_instance();
	};

	template<class DrawingSurface>
	auto generate_noisy_texture()
	{
		terraformer::image img{256, 256};
		terraformer::random_generator rng;
		std::uniform_real_distribution U{0.9375f, 1.0f/0.9375f};

		for(uint32_t y = 0; y != img.height(); ++y)
		{
			for(uint32_t x = 0; x != img.width(); ++x)
			{
				img(x, y) = 0.9375f*U(rng)*rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f};
				img(x, y).alpha(1.0f);
			}
		}

		return std::move(DrawingSurface{}.upload(std::as_const(img).pixels(), 8));
	}

	template<class DrawingSurface>
	auto generate_test_pattern()
	{
		terraformer::image img{800, 4*96};

		for(uint32_t y = 0; y != img.height(); ++y)
		{
			for(uint32_t x = 0; x != img.width(); ++x)
			{
				auto const row = y/16;
				auto const col = x/32;

				auto const xi = static_cast<float>(x)/static_cast<float>(img.width());
				auto const eta = static_cast<float>(y)/static_cast<float>(img.height());

				if((row%2 == 0 && col%2 == 1) || (row%2 == 1 && col%2 == 0))
				{ img(x, y) = xi*eta*rgba_pixel{0.5f, 0.5f, 0.5f, 1.0f}; }
				else
				{ img(x, y) = xi*eta*rgba_pixel{0.25f, 0.25f, 0.25f, 1.0f}; }
			}
		}

		return std::move(DrawingSurface{}.upload(std::as_const(img).pixels(), 8));
	}

	template<class DrawingSurface>
	auto generate_transparent_texture()
	{
		terraformer::image img{1, 1};
		img(0, 0) = rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f};
		return std::move(DrawingSurface{}.upload(std::as_const(img).pixels(), 0));
	}

	template<class DrawingSurface>
	auto generate_white_texture()
	{
		terraformer::image img{1, 1};
		img(0, 0) = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f};
		return std::move(DrawingSurface{}.upload(std::as_const(img).pixels(), 0));
	}

	template<class DrawingSurface>
	auto generate_default_stock_textures()
	{
		return default_stock_textures_repo{
			.none = generate_transparent_texture<DrawingSurface>(),
			.main_panel_background = generate_noisy_texture<DrawingSurface>(),
			.other_panel_background = generate_noisy_texture<DrawingSurface>(),
			.input_area_background = generate_white_texture<DrawingSurface>(),
			.command_area_background = generate_white_texture<DrawingSurface>(),
			.output_area_background = generate_white_texture<DrawingSurface>()
		};
	}
};

#endif