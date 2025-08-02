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
		PixelVisitor&& pixel_visitor
	)
	{
		auto const p1 = (seg.p1 - location{0.0f, 0.0f, 0.0f})/pixel_size;
		auto const p2 = (seg.p2 - location{0.0f, 0.0f, 0.0f})/pixel_size;
		auto const total_displacement = p2 - p1;
		auto const rep = total_displacement.get().get();
		auto const absvec = rep < geosimd::vec_t<float, 4>{}.get()? -rep : rep;
		auto const projected_distance = std::max(absvec[0], absvec[1]);
		auto const dr = total_displacement/projected_distance;
		auto const n = static_cast<size_t>(projected_distance);

		for(size_t k = 0; k != n; ++k)
		{
			auto const r = p1 + (static_cast<float>(k))*dr;
			pixel_visitor(r[0], r[1]);
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
			visit_pixels(
				geosimd::line_segment{.p1 = prev, .p2 = current},
				pixel_size,
				pixel_visitor
			);

			prev = current;
		}
	}
}
#endif
