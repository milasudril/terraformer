#ifndef TERRAFORER_UI_THEMING_THEME_LOADER_HPP
#define TERRAFORER_UI_THIMING_THEME_LOADER_HPP

#include "./texture_generators.hpp"
#include "./color_scheme.hpp"

#include "ui/font_handling/font_mapper.hpp"
#include "ui/font_handling/text_shaper.hpp"

namespace terraformer::ui::theming
{
	main::config load_default_config()
	{
		auto noisy_texture = std::make_shared<main::staged_texture const>(generate_noisy_texture());
		auto white_texture = std::make_shared<main::staged_texture const>(generate_white_texture());
		auto test_pattern = std::make_shared<main::staged_texture const>(generate_test_pattern());
		auto null_texture = std::make_shared<main::staged_texture const>(generate_null_texture());
		auto black_texture = std::make_shared<main::staged_texture const>(generate_black_texture());

		font_handling::font_mapper fonts;
		auto const fontfile = fonts.get_path("sans-serif");
		auto const body_text = std::make_shared<font_handling::font>(fontfile);
		body_text->set_font_size(11);

		auto const& color_scheme = default_color_scheme;

		return main::config{
			.main_panel{
				.colors = color_scheme.main_panel,
				.background_texture = noisy_texture,
				.padding = 4.0f
			},
			.other_panel{
				.colors = color_scheme.other_panel,
				.background_texture = noisy_texture,
				.padding = 4.0f
			},
			.input_area{
				.colors = color_scheme.input_area,
				.padding = 3.0f,
				.border_thickness = 1.0f,
				.font = body_text
			},
			.command_area{
				.colors = color_scheme.command_area,
				.padding = 2.0f,
				.border_thickness = 2.0f,
				.font = body_text
			},
			.output_area{
				.colors = color_scheme.output_area,
				.padding = 4.0f,
				.border_thickness = 0.0f,
				.font = body_text
			},
			.status_indicator = color_scheme.status_indicator,
			.mouse_kbd_tracking{
				.colors = color_scheme.mouse_kbd_tracking,
				.border_thickness = 4.0f
			},
			.misc_colors = color_scheme.misc_colors,
			.misc_textures{
				.null = null_texture,
				.black = black_texture,
				.white = white_texture,
				.test_pattern = test_pattern
			}
		};
	}
};

#endif
