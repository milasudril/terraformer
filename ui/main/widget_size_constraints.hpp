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
		std::optional<float> aspect_ratio;
	};

	constexpr widget_size_constraints max(widget_size_constraints const& a, widget_size_constraints const& b)
	{
		auto const width_range = merge(a.width, b.width);
		auto const height_range = merge(a.height, b.height);

		std::optional<float> aspect_ratio{};
		if(a.aspect_ratio.has_value() || b.aspect_ratio.has_value())
		{
			auto const n = static_cast<int>(a.aspect_ratio.has_value()) + static_cast<int>(b.aspect_ratio.has_value());
			aspect_ratio = (n == 1)?
					a.aspect_ratio.value_or(1.0) * b.aspect_ratio.value_or(1.0f):
					std::sqrt((*a.aspect_ratio) * (*b.aspect_ratio));
		}

		return widget_size_constraints{
			.width = width_range,
			.height = height_range,
			.aspect_ratio = aspect_ratio
		};
	}

	inline scaling minimize_height(widget_size_constraints const& constraints)
	{
		auto const preliminary_height = constraints.height.min();
		if(constraints.aspect_ratio.has_value())
		{
			auto const width = std::clamp(
				*constraints.aspect_ratio*preliminary_height,
				constraints.width.min(),
				constraints.width.max()
			);

			auto const new_height = width/(*constraints.aspect_ratio);
			if(new_height < constraints.height.min() || new_height > constraints.height.max())
			{ throw std::runtime_error{"Impossible size constraint"}; }

			return scaling{width, new_height, 1.0f};
		}

		return scaling{constraints.width.min(), preliminary_height, 1.0f};
	};

	inline scaling minimize_width(widget_size_constraints const& constraints)
	{
		auto const preliminary_width = constraints.width.min();
		if(constraints.aspect_ratio.has_value())
		{
			auto const height = std::clamp(
				preliminary_width/(*constraints.aspect_ratio),
				constraints.height.min(),
				constraints.height.max()
			);

			auto const new_width = height*(*constraints.aspect_ratio);

			if(new_width < constraints.width.min() || new_width > constraints.width.max())
			{ throw std::runtime_error{"Impossible size constraint"}; }

			return scaling{new_width, height, 1.0f};
		}

		return scaling{preliminary_width, constraints.height.min(), 1.0f};
	}
}
#endif