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

	struct curve_render_mask_descriptor
	{
		float pixel_size;
		float radius;
	};

	void render_mask(
		span_2d<bool> output,
		span<location const> curve,
		curve_render_mask_descriptor params
	);


	struct closest_point_info
	{
		float curve_parameter;
		float distance;
	};

	struct spline_with_length:public multi_array<polynomial<displacement, 3>, line_segment, float>
	{
		auto polynomials() const
		{ return get<0>(); }

		auto line_segments() const
		{ return get<1>(); }

		auto curve_lengths() const
		{ return get<2>(); }
	};

	float curve_length(
		polynomial<displacement, 3> const& curve,
		float t_start,
		float t_end,
		size_t seg_count
	);

	spline_with_length make_spline_with_lengths(span<location const> curve);

	single_array<polynomial<displacement, 3>> make_spline(span<location const> curve);

	closest_point_info
	find_closest_point(polynomial<displacement, 3> const& curve, line_segment input_seg, location loc);

	closest_point_info
	find_closest_point(spline_with_length const& curve, location loc);

	closest_point_info find_closest_point(span<location const> curve, location loc);

	template<class T>
	concept curve_shader = requires(T obj, closest_point_info point_info){
		{obj(point_info)};
	};

	template<curve_shader CurveShader>
	struct curve_render_descriptor
	{
		float pixel_size;
		span_2d<bool const> fill_mask;
		CurveShader shader;
	};

	template<curve_shader CurveShader>
	void render(
		scanline_processing_job_info const& jobinfo,
		span_2d<std::invoke_result_t<CurveShader, closest_point_info>> output,
		spline_with_length const& curve,
		curve_render_descriptor<CurveShader>&& params
	)
	{
		auto const y_offset = jobinfo.input_y_offset;
		auto const fill_mask = params.fill_mask;
		displacement const v{
			0.5f,
			static_cast<float>(y_offset) + 0.5f,
			0.0f
		};
		auto const pixel_size = params.pixel_size;

		for(uint32_t y = 0; y != output.height(); ++y)
		{
			for(uint32_t x = 0; x != output.width(); ++x)
			{
				if(fill_mask(x, y + y_offset))
				{
					auto const loc =
							location{}
						+ pixel_size*(displacement{static_cast<float>(x), static_cast<float>(y), 0.0f} + v );

					output(x, y) = params.shader(find_closest_point(curve, loc));
				}
			}
		}
	}

	template<curve_shader CurveShader>
	void render(
		span_2d<std::invoke_result_t<CurveShader, closest_point_info>> output,
		span<location const> curve,
		curve_render_descriptor<CurveShader>&& params
	)
	{
		auto const spline = make_spline_with_lengths(curve);
		render(
			scanline_processing_job_info{
				.input_y_offset = 0,
				.total_height = output.height()
			},
			output,
			spline,
			std::move(params)
		);
	}

	template<curve_shader CurveShader>
	struct curve_render_with_automask_descriptor
	{
		float pixel_size;
		float mask_radius;
		CurveShader shader;
	};

	template<curve_shader CurveShader>
	void render(
		span_2d<std::invoke_result_t<CurveShader, closest_point_info>> output,
		span<location const> curve,
		curve_render_with_automask_descriptor<CurveShader>&& params
	)
	{
		auto mask = create_with_same_size<bool>(output);
		auto const pixel_size = params.pixel_size;
		auto const mask_radius = params.mask_radius;
		render_mask(
			mask.pixels(),
			curve,
			curve_render_mask_descriptor{
				.pixel_size = pixel_size,
				.radius = mask_radius
			}
		);

		render(
			output,
			curve,
			curve_render_descriptor{
				.pixel_size = pixel_size,
				.fill_mask = mask.pixels(),
				.shader = std::move(params.shader)
			}
		);
	}
}
#endif
