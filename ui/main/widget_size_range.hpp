#ifndef TERRAFORMER_UI_MAIN_WIDGET_SIZE_RANGE_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_SIZE_RANGE_HPP

#include <limits>

namespace terraformer::ui::main
{
	struct widget_size_range
	{
		float min = 0.0f;
		float max = std::numeric_limits<float>::infinity();
	};
}

#endif