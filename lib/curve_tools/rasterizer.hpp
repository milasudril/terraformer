#ifndef TERRAFORMER_CURVE_RASTERIZER_HPP
#define TERRAFORMER_CURVE_RASTERIZER_HPP

#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"

#include <geosimd/line.hpp>

#include <optional>

namespace terraformer
{
	template<class T, class PixelType>
	concept brush = requires(T obj, PixelType old_val, PixelType new_val, float x, float y, float z, float xi, float eta)
	{
		{ obj.begin_pixel(x, y, z) } -> std::same_as<void>;
		{ std::as_const(obj).get_radius() } -> std::same_as<float>;
		{ std::as_const(obj).get_pixel_value(old_val, new_val, xi, eta) } -> std::same_as<PixelType>;
	};

	template<class PixelType, brush<PixelType> Brush>
	struct paint_params
	{
		float x;
		float y;
		PixelType value;
		Brush brush;
	};

	template<class PixelType, brush<PixelType> Brush>
	void paint(span_2d<PixelType> target_surface, paint_params<PixelType, Brush> const& params)
	{
		auto const h = target_surface.width();
		auto const w = target_surface.height();

		auto const r = params.brush.get_radius();
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
				auto const src_val = target_surface((l + l_min + w)%w, (k + k_min + h)%h);
				static_assert(std::is_same_v<decltype(l + l_min), uint32_t>);
				target_surface((l + l_min + w)%w, (k + k_min + h)%h ) =
					params.brush.get_pixel_value(src_val, new_val, xi, eta);
			}
		}
	}

	template<class PixelType, brush<PixelType> Brush>
	struct line_segment_draw_params
	{
		PixelType value;
		float scale;
		Brush brush{};
	};

	template<class PixelType, brush<PixelType> Brush>
	void draw(span_2d<PixelType> target_surface,
		geosimd::line_segment<geom_space> seg,
		line_segment_draw_params<PixelType, Brush>& params)
	{
		auto dr = seg.p2 - seg.p1;
		if(std::abs(dr[0]) > std::abs(dr[1]))
		{
			auto const a = dr[1]/dr[0];
			auto const b = dr[2]/dr[0];
			auto const dx = dr[0] >= 0.0f ? 1 : -1;
			for(auto l = static_cast<int32_t>(seg.p1[0]);
				l != static_cast<int32_t>(seg.p2[0]) + dx;
				l += dx)
			{
				auto const x = static_cast<float>(l);
				auto const y = a*static_cast<float>(l - static_cast<int32_t>(seg.p1[0]))
					+ seg.p1[1];
				auto const z = b*static_cast<float>(l - static_cast<int32_t>(seg.p1[0])) + seg.p1[2];
				params.brush.begin_pixel(x, y, z);
				paint(target_surface,
					paint_params<PixelType, Brush>{
						.x = x/params.scale,
						.y = y/params.scale,
						.value = z*params.value,
						.brush = params.brush
					}
				);
			}
		}
		else
		{
			auto const a = dr[0]/dr[1];
			auto const b = dr[2]/dr[1];
			auto const dy = dr[1] >= 0.0f ? 1 : -1;
			for(auto k = static_cast<int32_t>(seg.p1[1]);
				k != static_cast<int32_t>(seg.p2[1]) + dy;
				k += dy)
			{
				auto const y = static_cast<float>(k);
				auto const x = a*static_cast<float>(k - static_cast<int32_t>(seg.p1[1]))
					+ seg.p1[0];
				auto const z = b*static_cast<float>(k - static_cast<int32_t>(seg.p1[1])) + seg.p1[2];
				params.brush.begin_pixel(x, y, z);
				paint(target_surface,
					paint_params<PixelType, Brush>{
						.x = x/params.scale,
						.y = y/params.scale,
						.value = z*params.value,
						.brush = params.brush
					}
				);
			}
		}
	}


	template<class PixelType, brush<PixelType> Brush>
	void draw(span_2d<PixelType> target_surface,
		std::span<location const> curve,
		line_segment_draw_params<PixelType, Brush>&& params)
	{
		if(std::size(curve) == 0)
		{ return; }

		auto prev = curve[0];
		for(size_t k = 1; k!=std::size(curve); ++k)
		{
			auto const current = curve[k];
			draw(target_surface, geosimd::line_segment{.p1 = prev, .p2 = current}, params);
			prev = current;
		}
	}
}

#endif
