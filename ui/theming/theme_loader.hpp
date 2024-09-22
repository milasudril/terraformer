#ifndef TERRAFORER_UI_THEMING_THEME_LOADER_HPP
#define TERRAFORER_UI_THIMING_THEME_LOADER_HPP

#include "./texture_generators.hpp"
#include "ui/main/generic_texture.hpp"
#include "./color_scheme.hpp"
#include "ui/font_handling/font_mapper.hpp"
#include "ui/font_handling/text_shaper.hpp"
#include "lib/common/object_tree.hpp"

namespace terraformer::ui::theming
{
	struct panel_look
	{
		panel_colors colors;
		main::generic_shared_texture background_texture;
		float padding;
	};

	struct widget_look
	{
		widget_colors colors;
		float padding;
		std::shared_ptr<font_handling::font const> font;
	};

	struct event_routing_look
	{
		event_routing_colors colors;
		float border_thickness;
	};

	struct ui_config
	{
		panel_look main_panel;
		panel_look other_panel;
		widget_look input_area;
		widget_look command_area;
		widget_look output_area;
		status_colors status_indicator;
		event_routing_look cursor_kbd_tracking;
		palettes misc_colors;
	};

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
					.insert<rgba_pixel>("text_color", default_color_scheme.command_area.text)
					.insert_link("font", body_text)
					.insert<float>("background_intensity", 1.0f))
				)
				.insert<object_dict>("output_area", std::move(object_dict{}
					.insert_link("font", body_text)
					.insert<rgba_pixel>("text_color", default_color_scheme.output_area.text))
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
