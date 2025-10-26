//@	{"dependencies_extra":[{"ref":"./rasterizer.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_CURVE_RASTERIZER_HPP
#define TERRAFORMER_CURVE_RASTERIZER_HPP

#include "./line_segment.hpp"

#include "lib/array_classes/single_array.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/array_classes/span.hpp"
#include "lib/array_classes/multi_array.hpp"
#include "lib/math_utils/polynomial.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/math_utils/quad_renderer.hpp"

#include <optional>
#include <cassert>

namespace terraformer
{
	struct closest_point_info
	{
		float curve_parameter;
		float distance;
	};

	struct curve_part
	{
		location p_0;
		location p_1;
		float seg_length;
		polynomial<displacement, 3> interpolator;
	};

	single_array<curve_part> make_spline(span<location const> curve);

	[[gnu::const]] float curve_length(
		polynomial<displacement, 3> const& curve,
		float t_start,
		float t_end,
		size_t seg_count
	);

	[[gnu::const]] closest_point_info
	find_closest_point(curve_part const& curve, location loc);

	[[gnu::const]] closest_point_info
	find_closest_point(span<curve_part const> curve, location loc);

	[[gnu::const]] closest_point_info find_closest_point(span<location const> curve, location loc);

	template<class CurveShader>
	void make_distance_field(
		scanline_processing_job_info const& jobinfo,
		span_2d<std::invoke_result_t<CurveShader, closest_point_info>> output,
		span<location const> curve,
		float pixel_size,
		CurveShader&& shader
	)
	{
		auto const spline = make_spline(curve);

		auto const y_offset = static_cast<float>(jobinfo.input_y_offset);
		displacement const v{
			0.5f,
			y_offset + 0.5f,
			0.0f
		};

		for(uint32_t y = 0; y != output.height(); ++y)
		{
			for(uint32_t x = 0; x != output.width(); ++x)
			{
				auto const loc =
						location{}
					+ pixel_size*(displacement{static_cast<float>(x), static_cast<float>(y), 0.0f} + v );

				output(x, y) = shader(find_closest_point(spline, loc));
			}
		}
	}

	template<class PixelVisitor>
	void visit_pixels(
		geosimd::line_segment<geom_space> seg,
		float pixel_size,
		PixelVisitor&& pixel_visitor
	)
	{
		assert(pixel_size > 0.0f);

		auto const p1 = location{} + (seg.p1 - location{})/pixel_size;
		auto const p2 = location{} + (seg.p2 - location{})/pixel_size;
		direction const tangent{p2 - p1};
		auto const normal = direction{(p2 - p1).rot_right_angle_z_right()};
		auto const total_displacement = p2 - p1;
		auto const rep = total_displacement.get().get();
		auto const absvec = rep < geosimd::vec_t<float, 4>{}.get()? -rep : rep;
		auto const projected_distance = std::max(absvec[0], absvec[1]);
		auto const dr = total_displacement/projected_distance;
		auto const n = static_cast<size_t>(projected_distance);

		for(size_t k = 0; k != n + 1; ++k)
		{ pixel_visitor(p1 + (static_cast<float>(k))*dr, tangent, normal); }
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
