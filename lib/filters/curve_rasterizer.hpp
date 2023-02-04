#ifndef TERRAFORMER_CURVE_RASTERIZER_HPP
#define TERRAFORMER_CURVE_RASTERIZER_HPP

#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"

#include <geosimd/line.hpp>

#include <optional>

namespace terraformer
{
	template<class T>
	concept brush = requires(T f, float xi, float eta)
	{
		{f(xi, eta)} -> std::same_as<float>;
	};

	template<class T>
	concept brush_size_modulator = requires(T f, float x, float y)
	{
		{f(x, y)} -> std::same_as<float>;
	};

	template<class T, class PixelType>
	concept blend_function = requires(T f, PixelType old_val, PixelType new_val, float brush_strength)
	{
		{f(old_val, new_val, brush_strength)} -> std::same_as<PixelType>;
	};

	struct solid_circle
	{
		constexpr auto operator()(float x, float y) const
		{ return x*x + y*y <= 1.0f ? 1.0f : 0.0f; }
	};

	struct default_brush_size_modulator
	{
		constexpr auto operator()(float, float) const
		{
			return 1.0f;
		}
	};

	template<class PixelType>
	struct lerp
	{
		constexpr auto operator()(PixelType old_val, PixelType new_val, float brush_strength) const
		{
			return brush_strength*new_val + (1.0f - brush_strength)*old_val;
		}
	};

	template<class PixelType,
		brush Brush = solid_circle,
		blend_function<PixelType> BlendFunction = lerp<PixelType>>
	struct paint_params
	{
		float x_0;
		float y_0;
		PixelType value;
		float brush_diameter;
		Brush brush;
		BlendFunction blend_function;
	};

	template<class PixelType, brush Brush = solid_circle>
	void draw(span_2d<PixelType> target_surface,
		float x,
		float y,
		PixelType value,
		float d,
		Brush&& brush = Brush{})
	{
		auto const h = target_surface.width();
		auto const w = target_surface.height();

		auto const thickness = 0.5f*d;
		auto const brush_size = static_cast<uint32_t>(d);
		auto const k_min = static_cast<uint32_t>(y - thickness + 0.5f);
		auto const l_min = static_cast<uint32_t>(x - thickness + 0.5f);

		for(auto k = 0u; k != brush_size; ++k)
		{
			for(auto l = 0u; l != brush_size; ++l)
			{
				auto const xi = std::lerp(-1.0f, 1.0f, (static_cast<float>(l) + 0.5f)/d);
				auto const eta = std::lerp(-1.0f, 1.0f, (static_cast<float>(k) + 0.5f)/d);

				auto const strength = brush(xi, eta);
				auto const src_val = target_surface((l + l_min + w)%w, (h - 1) - (k + k_min + h)%h);
				target_surface((l + l_min + w)%w, (h - 1) - (k + k_min + h)%h ) = strength*value + (1.0f - strength)*src_val;
			}
		}
	}

	template<class PixelType,
		brush_size_modulator BrushSizeModulator = default_brush_size_modulator,
		brush Brush = solid_circle>
	void draw(span_2d<PixelType> target_surface,
		geosimd::line_segment<geom_space> seg,
		PixelType value,
		BrushSizeModulator&& mod = BrushSizeModulator{},
		Brush&& brush = Brush{})
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
				auto const d = mod(x, y);
				draw(target_surface, x, y, z*value, d, brush);
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
				auto const d = mod(x, y);
				draw(target_surface, x, y, z*value, d, brush);
			}
		}
	}

	template<class PixelType,
		brush_size_modulator BrushSizeModulator = default_brush_size_modulator,
		brush Brush = solid_circle>
	void draw_as_line_segments(span_2d<PixelType> target_surface,
		std::span<location const> curve,
		PixelType value,
		BrushSizeModulator&& mod = BrushSizeModulator{},
		Brush&& brush = Brush{})
	{
		if(std::size(curve) == 0)
		{ return; }

		auto prev = curve[0];
		for(size_t k = 1; k!=std::size(curve); ++k)
		{
			auto const current = curve[k];
			draw(target_surface, geosimd::line_segment{.p1 = prev, .p2 = current}, value, mod, brush);
			prev = current;
		}
	}

	template<class PixelType,
		brush_size_modulator BrushSizeModulator = default_brush_size_modulator,
		brush Brush = solid_circle>
	void draw_as_dots(span_2d<PixelType> target_surface,
		std::span<location const> curve,
		PixelType value,
		BrushSizeModulator&& mod = BrushSizeModulator{},
		Brush&& brush = Brush{})
	{
		for(auto point: curve)
		{
			draw(target_surface, point, value, mod, brush);
		}
	}

}

#endif
