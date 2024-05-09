#ifndef TERRAFORMER_UI_THEMING_WIDGET_LOOK_HPP
#define TERRAFORMER_UI_THEMING_WIDGET_LOOK_HPP

#include "./color_scheme.hpp"

namespace terraformer::ui::theming
{
	struct frame_look
	{
		float thickness;
	};

	constexpr frame_look default_interactive_frame_look{
		.thickness = 8.0f,
	};

	struct widget_look
	{
		color_scheme colors;
		frame_look interactive_frame;
	};

	constexpr widget_look default_widget_look{
		.colors = default_color_scheme,
		.interactive_frame = default_interactive_frame_look
	};
}

#endif