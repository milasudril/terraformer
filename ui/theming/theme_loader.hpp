#ifndef TERRAFORER_UI_THEMING_THEME_LOADER_HPP
#define TERRAFORER_UI_THIMING_THEME_LOADER_HPP

#include "./texture_generators.hpp"
#include "./color_scheme.hpp"

#include "ui/drawing_api/image_generators.hpp"
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
		auto const fontfile = fonts.get_path("Andika, Verdana, Comic Sans MS, sans-serif");
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
				.min_img_height = 240.0f,
				.font = body_text
			},
			.command_area{
				.colors = color_scheme.command_area,
				.padding = 2.0f,
				.border_thickness = 2.0f,
				.min_img_height = 240.0f,
				.font = body_text
			},
			.output_area{
				.colors = color_scheme.output_area,
				.padding = 4.0f,
				.border_thickness = 1.0f,
				.min_img_height = 240.0f,
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
				.test_pattern = test_pattern,
				.horizontal_handle = std::make_shared<main::staged_texture>(
					generate(
						drawing_api::beveled_rectangle{
							.domain_size = span_2d_extents{
								.width = 16,
								.height = 8,
							},
							.origin_x = 0,
							.origin_y = 0,
							.width = 16,
							.height = 8,
							.border_thickness = 2,
							.upper_left_color = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
							.lower_right_color = rgba_pixel{0.25f, 0.25f, 0.25f, 1.0f},
							.fill_color = rgba_pixel{0.5f, 0.5f, 0.5f, 1.0f},
						}
					)
				),
				.vertical_handle = std::make_shared<main::staged_texture>(
					generate(
						drawing_api::beveled_rectangle{
							.domain_size = span_2d_extents{
								.width = 8,
								.height = 16,
							},
							.origin_x = 0,
							.origin_y = 0,
							.width = 8,
							.height = 16,
							.border_thickness = 2,
							.upper_left_color = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
							.lower_right_color = rgba_pixel{0.25f, 0.25f, 0.25f, 1.0f},
							.fill_color = rgba_pixel{0.5f, 0.5f, 0.5f, 1.0f},
						}
					)
				),
				.small_knob = std::make_shared<main::staged_texture>(
					generate(
						drawing_api::beveled_disc{
							.domain_size{
								.width = 32,
								.height = 32
							},
							.origin_x = 0,
							.origin_y = 0,
							.radius = 16,
							.border_thickness = 2,
							.upper_left_color = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
							.lower_right_color = rgba_pixel{0.25f, 0.25f, 0.25f, 1.0f},
							.fill_color = rgba_pixel{0.5f, 0.5f, 0.5f, 1.0f},
						}
					)
				),
				.small_hand = std::make_shared<main::staged_texture>(
					generate(
						drawing_api::hand{
							.domain_size = span_2d_extents{
								.width = 32,
								.height = 32,
							},
							.thickness = 2,
							.length = 13,
							.color = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
						}
					)
				),
			}
		};
	}
}
#endif
