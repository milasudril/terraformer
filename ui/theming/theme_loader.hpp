#ifndef TERRAFORER_UI_THEMING_THEME_LOADER_HPP
#define TERRAFORER_UI_THIMING_THEME_LOADER_HPP

#include "./texture_generators.hpp"
#include "./color_scheme.hpp"
#include "lib/common/object_tree.hpp"

namespace terraformer::ui::theming
{
	template<class TextureType>
	object_dict load_default_resources()
	{
		shared_any noisy_texture{
			std::type_identity<TextureType>{},
			generate_noisy_texture<TextureType>()
		};

		object_dict resources;
		resources.insert<object_dict>(
			"ui", object_dict{}
				.insert<object_array>("panels", object_array{}
					.append<object_dict>(object_dict{}
						.insert_link("background_texture", noisy_texture)
						.insert<rgba_pixel>("background_tint", default_color_scheme.main_panel.background)
					)
				)
				.insert<object_array>("misc_dark_colors", object_array{}
					.append<rgba_pixel>(1.0f, 0.0f, 0.0f, 1.0f)
				)
				.insert<TextureType>(
					"null_texture",
					generate_transparent_texture<TextureType>()
				)
			);
		return resources;
	}
};

#endif