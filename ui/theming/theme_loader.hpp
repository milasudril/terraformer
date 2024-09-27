#ifndef TERRAFORER_UI_THEMING_THEME_LOADER_HPP
#define TERRAFORER_UI_THIMING_THEME_LOADER_HPP

#include "./texture_generators.hpp"
#include "./color_scheme.hpp"

#include "ui/main/generic_texture.hpp"
#include "ui/font_handling/font_mapper.hpp"
#include "ui/font_handling/text_shaper.hpp"
#include "lib/common/object_tree.hpp"

namespace terraformer::ui::theming
{
	template<class TextureType>
	main::config load_default_config()
	{
		main::generic_shared_texture const noisy_texture{
			std::type_identity<TextureType>{},
			generate_noisy_texture<TextureType>()
		};

		main::generic_shared_texture const white_texture{
			std::type_identity<TextureType>{},
			generate_white_texture<TextureType>()
		};

		main::generic_shared_texture const test_pattern{
			std::type_identity<TextureType>{},
			generate_test_pattern<TextureType>()
		};

		main::generic_shared_texture const null_texture{
			std::type_identity<TextureType>{},
			generate_transparent_texture<TextureType>()
		};

		main::generic_shared_texture const black_texture{
			std::type_identity<TextureType>{},
			generate_black_texture<TextureType>()
		};

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
				.padding = 2.0f,
				.border_thickness = 2.0f,
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

	template<class TextureType>
	object_dict load_default_resources()
	{
		shared_any noisy_texture{
			std::type_identity<main::generic_unique_texture>{},
			std::type_identity<TextureType>{},
			generate_noisy_texture<TextureType>()
		};

		shared_any white_texture{
			std::type_identity<main::generic_unique_texture>{},
			std::type_identity<TextureType>{},
			generate_white_texture<TextureType>()
		};

		font_handling::font_mapper fonts;
 		auto const fontfile = fonts.get_path("sans-serif");

		shared_any body_text{
			std::type_identity<font_handling::font>{},
			std::move(font_handling::font{fontfile.c_str()}.set_font_size(11))
		};

		object_dict resources;
		resources.insert<object_dict>(
			"ui", std::move(object_dict{}
				.insert<object_array>("panels", std::move(object_array{}
					.append<object_dict>(std::move(object_dict{}
						.insert_link("background_texture", noisy_texture)
						.insert<rgba_pixel>("background_tint", default_color_scheme.main_panel.background)
						.insert<object_dict>("margins", std::move(object_dict{}
							.insert<float>("x", 4.0f)
							.insert<float>("y", 4.0f))
						))
					))
				)
				.insert<object_dict>("command_area", std::move(object_dict{}
					.insert<rgba_pixel>("background_tint", default_color_scheme.command_area.background)
					.insert<rgba_pixel>("text_color", default_color_scheme.command_area.foreground)
					.insert_link("font", body_text)
					.insert<float>("background_intensity", 1.0f))
				)
				.insert<object_dict>("output_area", std::move(object_dict{}
					.insert_link("font", body_text)
					.insert<rgba_pixel>("text_color", default_color_scheme.output_area.foreground))
				)
				.insert<unsigned int>("widget_inner_margin", 4)
				.insert<unsigned int>("3d_border_thickness", 2)
				.insert<main::generic_unique_texture>(
					"null_texture",
					std::type_identity<TextureType>{},
					generate_transparent_texture<TextureType>()
				)
			)
		);
		return resources;
	}
};

#endif
