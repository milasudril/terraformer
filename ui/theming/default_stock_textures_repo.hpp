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

		DrawingSurface clean;
		DrawingSurface noisy;

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
			.clean = generate_white_texture<DrawingSurface>(),
			.noisy = generate_noisy_texture<DrawingSurface>()
		};
	}
};

#endif