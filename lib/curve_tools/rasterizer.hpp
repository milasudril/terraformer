//@	{"dependencies_extra":[{"ref":"./rasterizer.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_CURVE_RASTERIZER_HPP
#define TERRAFORMER_CURVE_RASTERIZER_HPP

#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/array_classes/span.hpp"
#include "lib/array_classes/multi_array.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/math_utils/quad_renderer.hpp"

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
		auto const p1 = location{} + (seg.p1 - location{})/pixel_size;
		auto const p2 = location{} + (seg.p2 - location{})/pixel_size;
		auto const total_displacement = p2 - p1;
		auto const rep = total_displacement.get().get();
		auto const absvec = rep < geosimd::vec_t<float, 4>{}.get()? -rep : rep;
		auto const projected_distance = std::max(absvec[0], absvec[1]);
		auto const dr = total_displacement/projected_distance;
		auto const n = static_cast<size_t>(projected_distance);

		for(size_t k = 0; k != n; ++k)
		{ pixel_visitor(p1 + (static_cast<float>(k))*dr); }
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

	struct thick_curve
	{
		struct vertex
		{
			location loc;
			direction normal;
			float thickness;
		};

		auto locations() const
		{ return data.get<0>(); }

		auto normals() const
		{ return data.get<1>(); }

		auto thicknesses() const
		{ return data.get<2>(); }

		multi_array<location, direction, float> data;
	};

	thick_curve make_thick_curve(span<location const> curve, span<float const> curve_thickness);

	template<class PixelType, class PerQuadShader>
	void fill_using_quads(
		span<location const> curve,
		float pixel_size,
		span<float const> curve_thickness,
		span_2d<PixelType> output_image,
		PerQuadShader&& shader
	)
	{
		auto const thick_curve = make_thick_curve(curve, curve_thickness);
		if(std::size(thick_curve.data).get() < 2)
		{ return; }

	}
}
#endif
