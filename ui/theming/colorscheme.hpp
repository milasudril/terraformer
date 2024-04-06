#ifndef TERRAFORMER_UI_THEMING_COLORSCHEME_HPP
#define TERRAFORMER_UI_THEMING_COLORSCHEME_HPP

#include "lib/pixel_store/rgba_pixel.hpp"

#include <array>

namespace terraformer::ui::theming
{
	struct color_set
	{
		rgba_pixel background;
		rgba_pixel text;
		rgba_pixel border;
	};

	struct color_scheme
	{
		color_set main_panel;
		color_set other_panel;
		color_set input_area;
		color_set command_area;
		color_set output_area;

		color_set selection;
		color_set mouse_focus;
		color_set keyboard_focus;

		std::array<rgba_pixel, 16> misc_colors;
	};

	inline constinit color_scheme current_color_scheme{
		.main_panel{
			.background = rgba_pixel{
				0.875f/2.0f,
				0.875f/2.0f,
				0.875f/2.0f,
				1.0f
			},
			.text = rgba_pixel{
				0.0f,
				0.0f,
				0.0f,
				1.0f
			},
			.border = rgba_pixel{
				0.0f,
				0.0f,
				0.0f,
				1.0f
			}
		},
		.other_panel{
		},
		.input_area{
			.background = rgba_pixel{
				0.875f,
				0.875f,
				0.875f,
				1.0f
			},
			.text = rgba_pixel{
				0.0f,
				0.0f,
				0.0f,
				1.0f
			},
			.border = rgba_pixel{
				0.0f,
				0.0f,
				0.0f,
				1.0f
			}
		},
		.command_area{
		},
		.output_area{
		},
		.selection{
		},
		.mouse_focus{
		},
		.keyboard_focus{
		},
		.misc_colors{}
	};
}

#endif