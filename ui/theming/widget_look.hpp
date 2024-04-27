#ifndef TERRAFORMER_UI_THEMING_WIDGET_LOOK_HPP
#define TERRAFORMER_UI_THEMING_WIDGET_LOOK_HPP

#include "./color_scheme.hpp"

namespace terraformer::ui::theming
{
	struct widget_look
	{
		color_scheme colors;
	};

	constexpr widget_look default_widget_look{
		.colors = default_color_scheme
	};
}

#endif