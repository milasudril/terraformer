#ifndef TERRAFORMER_CURVE_RASTERIZER_HPP
#define TERRAFORMER_CURVE_RASTERIZER_HPP

#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/array_classes/span.hpp"
#include "lib/pixel_store/image.hpp"

#include <geosimd/line.hpp>

#include <optional>
#include <cassert>

namespace terraformer
{
	template<class PixelVisitor>
	void visit_pixels(
		geosimd::line_segment<geom_space> seg,
		float pixel_size,
		PixelVisitor&& pixel_visitor,
		array_index<location> starting_at
	)
	{
		auto const p1 = (seg.p1 - location{0.0f, 0.0f, 0.0f})/pixel_size;
		auto const p2 = (seg.p2 - location{0.0f, 0.0f, 0.0f})/pixel_size;
		auto const dr = p2 - p1;
		if(std::abs(dr[0]) > std::abs(dr[1]))
		{
			auto const a = dr[1]/dr[0];
			auto const dx = dr[0] >= 0.0f ? 1 : -1;
			for(auto l = static_cast<int32_t>(p1[0]);
				l != static_cast<int32_t>(p2[0]);
				l += dx)
			{
				auto const y = a*static_cast<float>(l - static_cast<int32_t>(p1[0])) + static_cast<int32_t>(p1[1]) + 0.5f;
				auto const x = static_cast<float>(l) + 0.5f;
				pixel_visitor(x, y, starting_at);
			}
		}
		else
		{
			auto const a = dr[0]/dr[1];
			auto const dy = dr[1] >= 0.0f ? 1 : -1;
			for(auto k = static_cast<int32_t>(p1[1]);
				k != static_cast<int32_t>(p2[1]);
				k += dy)
			{
				auto const x = a*static_cast<float>(k - static_cast<int32_t>(p1[1])) + static_cast<int32_t>(p1[0]) + 0.5f;
				auto const y = static_cast<float>(k) + 0.5f;
				pixel_visitor(x, y, starting_at);
			}
		}
	}

	template<class PixelVisitor>
	void visit_pixels(
		span<location const> curve,
		float pixel_size,
		PixelVisitor&& pixel_visitor
	)
	{
		if(curve.empty())
		{ return; }

		// TODO: C++23 adjacent_view
		auto prev = curve.front();
		for(auto k : curve.element_indices(1))
		{
			auto const current = curve[k];
			visit_pixels(geosimd::line_segment{.p1 = prev, .p2 = current}, pixel_size, pixel_visitor, k);
			prev = current;
		}
	}
}
#endif
