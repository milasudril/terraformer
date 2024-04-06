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

	struct colorscheme
	{
		color_set main_panel;
		color_set other_panel;
		color_set input_area;
		color_set command_area;
		color_set output_area;

		color_set selection;
		color_set mouse_focus;
		color_set keybord_focus;

		std::array<rgba_pixel, 16> misc_colors;
	};
}

#endif