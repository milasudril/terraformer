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
	struct closest_point_info
	{
		float curve_parameter;
		float distance;
	};

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

				output(x, y) = shader(find_closest_point(curve, loc));
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

	struct thick_curve_view
	{
		struct vertex
		{
			location loc;
			direction normal;
			float thickness;
			float running_length;
		};


		auto locations() const
		{ return data.get<0>(); }

		auto normals() const
		{ return data.get<1>(); }

		auto thicknesses() const
		{ return data.get<2>(); }

		auto running_lengths() const
		{ return data.get<3>(); }

		multi_span<location const, direction const, float const, float const> data;
		float curve_length = 0.0f;
	};

	struct thick_curve
	{
		using vertex = thick_curve_view::vertex;

		auto locations() const
		{ return data.get<0>(); }

		auto normals() const
		{ return data.get<1>(); }

		auto thicknesses() const
		{ return data.get<2>(); }

		auto running_lengths() const
		{ return data.get<3>(); }

		auto attributes() const
		{
			return thick_curve_view{
				.data = data.attributes(),
				.curve_length = curve_length
			};

		}

		multi_array<location, direction, float, float> data;
		float curve_length = 0.0f;
	};

	thick_curve make_thick_curve(span<location const> curve, span<float const> curve_thickness);

	template<class PixelType, class Shader>
	void fill_using_quads(
		thick_curve_view curve,
		float pixel_size,
		span_2d<PixelType> output_image,
		Shader&& shader
	)
	{
		if(std::size(curve.data).get() < 2)
		{ return; }

		auto const elems = curve.data.element_indices(1);
		auto const locs  = curve.locations();
		auto const normals = curve.normals();
		auto const thicknesses = curve.thicknesses();
		auto const running_lenghts = curve.running_lengths();

		thick_curve_view::vertex last_vertex{
			.loc = locs.front(),
			.normal = normals.front(),
			.thickness = thicknesses.front(),
			.running_length = running_lenghts.front()
		};

		for(auto item: elems)
		{
			auto const current_loc = locs[item];
			auto const current_normal = normals[item];
			auto const current_thickness = thicknesses[item];
			auto const current_running_length = running_lenghts[item];

			auto const prev_loc = last_vertex.loc;
			auto const prev_normal = last_vertex.normal;
			auto const prev_thickness = last_vertex.thickness;
			auto const prev_running_length = last_vertex.running_length;

			auto const prev_v = prev_thickness*prev_normal;
			auto const current_v = current_thickness*current_normal;

			auto const origin = prev_loc - prev_v;
			auto const lower_right = prev_loc + prev_v;
			auto const upper_left = current_loc - current_v;
			auto const remote = current_loc + current_v;

			auto const intersect = intersect_2d(
				geosimd::line{
					.p1 = origin,
					.p2 = lower_right
				},
				geosimd::line{
					.p1 = upper_left,
					.p2 = remote
				}
			);
			if(intersect.has_value() && (intersect->a.get() >= 0.0f && intersect->a.get() <= 1.0f))
			{
				fprintf(stderr, "FIXME: Curve needs to be cleaned up again\n");
				fflush(stderr);
				abort();
			}

			auto const segment_length = distance(current_loc, prev_loc);
			render_quad(
				quad{
					.origin = location{} + (origin - location{})/pixel_size,
					.lower_right = location{} + (lower_right - location{})/pixel_size,
					.upper_left = location{} + (upper_left - location{})/pixel_size,
					.remote = location{} + (remote - location{})/pixel_size
				},
				output_image,
				[prev_running_length, segment_length, &shader](location loc) {
					auto const loc_transformed =
						  location{0.0f, prev_running_length, 0.0f}
						+ (loc - location{0.5f, 0.0f, 0.0f}).apply(scaling{2.0f, segment_length, 1.0f});
					return shader(loc_transformed);
				}
			);

			last_vertex = thick_curve::vertex{
				.loc = current_loc,
				.normal = current_normal,
				.thickness = current_thickness,
				.running_length = current_running_length
			};
		}
	}
}
#endif
