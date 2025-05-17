#ifndef TERRAFORMER_UI_MAIN_WIDGET_GEOMETRY_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_GEOMETRY_HPP

#include "./events.hpp"

#include "lib/common/spaces.hpp"

namespace terraformer::ui::main
{
	struct widget_geometry
	{
		location where;
		location origin;
		box_size size;

		[[nodiscard]] constexpr bool operator==(widget_geometry const&) const = default;
		[[nodiscard]] constexpr bool operator!=(widget_geometry const&) const = default;
	};

	[[nodiscard]] inline bool inside(location pos, widget_geometry const& box)
	{
		auto const r = to_scaling(0.5*box.size);
		auto const offset_to_origin = (location{0.0f, 0.0f, 0.0f} - box.origin).apply(r);
		auto const object_midpoint = box.where + offset_to_origin;
		auto const dr = pos - object_midpoint;
		return std::abs(dr[0]) < r[0] && std::abs(dr[1]) < r[1];
	}
}

#endif