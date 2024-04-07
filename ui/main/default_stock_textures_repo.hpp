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

		DrawingSurface null_texture;
		DrawingSurface main_panel_background;
		DrawingSurface other_panel_background;
		DrawingSurface input_area_background;
		DrawingSurface command_area_background;
		DrawingSurface output_area_backround;

		[[nodiscard]] inline static auto& get_default_instance();
	};

	template<class DrawingSurface>
	auto generate_default_background(rgba_pixel color)
	{
		terraformer::image img{256, 256};
		terraformer::random_generator rng;
		std::uniform_real_distribution U{0.9375f, 1.0f/0.9375f};

		for(uint32_t y = 0; y != img.height(); ++y)
		{
			for(uint32_t x = 0; x != img.width(); ++x)
			{
				img(x, y) = 0.9375f*U(rng)*color;
				img(x, y).alpha(1.0f);
			}
		}

		return std::move(DrawingSurface{}.upload(std::as_const(img).pixels(), 8));
	}

	template<class DrawingSurface>
	auto generate_null_texture()
	{
		terraformer::image img{1, 1};
		return std::move(DrawingSurface{}.upload(std::as_const(img).pixels(), 0));
	}

	template<class DrawingSurface>
	auto generate_default_stock_textures(theming::color_scheme const& color_scheme = theming::current_color_scheme)
	{
		return default_stock_textures_repo{
			.null_texture = generate_null_texture<DrawingSurface>(),
			.main_panel_background = generate_default_background<DrawingSurface>(color_scheme.main_panel.background),
			.other_panel_background = generate_default_background<DrawingSurface>(color_scheme.other_panel.background),
			.input_area_background = generate_default_background<DrawingSurface>(color_scheme.input_area.background),
			.command_area_background = generate_default_background<DrawingSurface>(color_scheme.command_area.background),
			.output_area_backround = generate_default_background<DrawingSurface>(color_scheme.output_area.background)
		};
	}

	template<class DrawingSurface>
	inline auto& default_stock_textures_repo<DrawingSurface>::get_default_instance()
	{
		thread_local auto ret = generate_default_stock_textures<DrawingSurface>();
		return ret;
	}
};

#endif