#ifndef TERRAFORMER_UI_MAIN_WIDGET_FRAME_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_FRAME_HPP

#include "./texture.hpp"

#include <array>

namespace terraformer::ui::main
{
	struct widget_frame
	{
		float thickness;
		texture_ref texture;
		std::array<rgba_pixel, 8> tints;
	};
}

#endif