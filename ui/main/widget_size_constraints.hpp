#ifndef TERRAFORMER_UI_WIDGET_SIZE_CONSTRAINTS_HPP
#define TERRAFORMER_UI_WIDGET_SIZE_CONSTRAINTS_HPP

#include "./widget_size_range.hpp"
#include "lib/common/spaces.hpp"

#include <optional>
#include <cmath>

namespace terraformer::ui::main
{
	struct widget_size_constraints
	{
		widget_size_range width;
		widget_size_range height;
	};

	constexpr widget_size_constraints merge(
		widget_size_constraints const& a,
		widget_size_constraints const& b
	)
	{
		return widget_size_constraints{
			.width = merge(a.width, b.width),
			.height = merge(a.height, b.height)
		};
	}

	constexpr scaling minimize_height(widget_size_constraints const& constraints)
	{ return scaling{constraints.width.min(), constraints.height.min(), 1.0f}; }

	constexpr scaling minimize_width(widget_size_constraints const& constraints)
	{ return scaling{constraints.width.min(), constraints.height.min(), 1.0f}; }
}
#endif