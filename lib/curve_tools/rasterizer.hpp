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
	template<class T, class PixelType, class PixelTypeOut>
	concept brush = requires(T obj, PixelType new_val, PixelTypeOut& old_val, float x, float y, float z, array_index<location> starting_at, float xi, float eta)
	{
		{ obj.begin_pixel(x, y, z, starting_at) } -> std::same_as<void>;
		{ std::as_const(obj).get_radius() } -> std::same_as<float>;
		{ std::as_const(obj).get_pixel_value(old_val, new_val, xi, eta) } -> std::same_as<void>;
	};

	template<class PixelType>
	struct paint_params
	{
		float x;
		float y;
		PixelType value;
	};

	template<class PixelTypeOut, class PixelType, brush<PixelType, PixelTypeOut> Brush>
	void paint(span_2d<PixelTypeOut> target_surface, paint_params<PixelType> const& params, Brush const& brush)
	{
		auto const h = target_surface.width();
		auto const w = target_surface.height();

		auto const r = brush.get_radius();
		assert(r >= 0.0f);
		auto const d = 2.0f*r;
		auto const brush_size = static_cast<uint32_t>(d);
		auto const k_min = static_cast<int32_t>(params.y - r + 0.5f);
		auto const l_min = static_cast<int32_t>(params.x - r + 0.5f);
		auto const new_val = params.value;

		for(auto k = 0u; k != brush_size; ++k)
		{
			for(auto l = 0u; l != brush_size; ++l)
			{
				auto const xi = std::lerp(-1.0f, 1.0f, (static_cast<float>(l) + 0.5f)/d);
				auto const eta = std::lerp(-1.0f, 1.0f, (static_cast<float>(k) + 0.5f)/d);
				brush.get_pixel_value(target_surface((l + l_min + w)%w, (k + k_min + h)%h), new_val, xi, eta);
			}
		}
	}

	template<class PixelType>
	struct line_segment_draw_params
	{
		PixelType value;
		float scale;
	};

	template<class PixelTypeOut, class PixelType, brush<PixelType, PixelTypeOut> Brush>
	void draw(span_2d<PixelTypeOut> target_surface,
		array_index<location> starting_at,
		geosimd::line_segment<geom_space> seg,
		line_segment_draw_params<PixelType> const& params,
		Brush& brush,
		span_2d<uint8_t> paint_mask
	)
	{
		auto const scale = params.scale;
		auto const p1 = (seg.p1 - location{0.0f, 0.0f, 0.0f})/scale;
		auto const p2 = (seg.p2 - location{0.0f, 0.0f, 0.0f})/scale;

		auto const dr = p2 - p1;
		auto const w = paint_mask.width();
		auto const h = paint_mask.height();

		if(std::abs(dr[0]) > std::abs(dr[1]))
		{
			auto const a = dr[1]/dr[0];
			auto const b = dr[2]/dr[0];
			auto const dx = dr[0] >= 0.0f ? 1 : -1;
			for(auto l = static_cast<int32_t>(p1[0]);
				l != static_cast<int32_t>(p2[0]) + dx;
				l += dx)
			{
				auto const y = a*static_cast<float>(l - static_cast<int32_t>(p1[0]))
					+ p1[1];
				auto const k = static_cast<uint32_t>(y);
				if(paint_mask((l + w)%w, (k + h)%h) != 1)
				{
					auto const x = static_cast<float>(l);
					auto const z = scale*(b*static_cast<float>(l - static_cast<int32_t>(p1[0])) + p1[2]);
					brush.begin_pixel(x, y, z, starting_at);
					paint(target_surface,
						paint_params{
							.x = x,
							.y = y,
							.value = z*params.value
						},
						brush
					);
					paint_mask((l + w)%w, (k + h)%h) = 1;
				}
			}
		}
		else
		{
			auto const a = dr[0]/dr[1];
			auto const b = dr[2]/dr[1];
			auto const dy = dr[1] >= 0.0f ? 1 : -1;
			for(auto k = static_cast<int32_t>(p1[1]);
				k != static_cast<int32_t>(p2[1]) + dy;
				k += dy)
			{
				auto const x = a*static_cast<float>(k - static_cast<int32_t>(p1[1]))
					+ p1[0];
				auto const l = static_cast<uint32_t>(x);
				if(paint_mask((l + w)%w, (k + h)%h) != 1)
				{
					auto const y = static_cast<float>(k);
					auto const z = scale*(b*static_cast<float>(k - static_cast<int32_t>(p1[1])) + p1[2]);
					brush.begin_pixel(x, y, z, starting_at);
					paint(target_surface,
						paint_params{
							.x = x,
							.y = y,
							.value = z*params.value
						},
						brush
					);
					paint_mask((l + w)%w, (k + h)%h) = 1;
				}
			}
		}
	}

	template<class PixelTypeOut, class PixelType, brush<PixelType, PixelTypeOut> Brush>
	void draw(span_2d<PixelTypeOut> target_surface,
		span<location const> curve,
		line_segment_draw_params<PixelType> const& params,
		Brush&& brush,
		span_2d<uint8_t> paint_mask
	)
	{
		if(curve.empty())
		{ return; }

		// TODO: C++23 adjacent_view
		auto prev = curve.front();
		for(auto k : curve.element_indices(1))
		{
			auto const current = curve[k];
			draw(target_surface, k, geosimd::line_segment{.p1 = prev, .p2 = current}, params, brush, paint_mask);
			prev = current;
		}
	}

	template<class PixelTypeOut, class PixelType, brush<PixelType, PixelTypeOut> Brush>
	void draw(span_2d<PixelTypeOut> target_surface,
		span<location const> curve,
		line_segment_draw_params<PixelType> const& params,
		Brush&& brush)
	{
		if(curve.empty())
		{ return; }

		auto paint_mask = create_with_same_size<uint8_t>(target_surface);
		draw(
			target_surface,
			curve,
			params,
			std::forward<Brush>(brush),
			paint_mask
		);
	}
}
#endif
