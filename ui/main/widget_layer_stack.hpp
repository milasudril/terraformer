#ifndef TERRAFORMER_UI_MAIN_WIDGET_LAYER_STACK_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_LAYER_STACK_HPP

#include "./texture.hpp"

namespace terraformer::ui::main
{
	struct widget_layer
	{
		displacement offset;
		texture_reference texture;
		std::array<rgba_pixel, 4> tints;
	};

	struct widget_layer_mask
	{
		displacement offset;
		texture_reference texture;
	};

	struct widget_layer_stack
	{
		widget_layer background;
		widget_layer_mask bg_mask;
		widget_layer selction_backround;
		widget_layer foreground;
		widget_layer frame;
		widget_layer input_marker;
	};
}

#endif