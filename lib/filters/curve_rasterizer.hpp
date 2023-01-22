#ifndef TERRAFORMER_CURVE_RASTERIZER_HPP
#define TERRAFORMER_CURVE_RASTERIZER_HPP

#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"

#include <geosimd/line.hpp>

#include <optional>

namespace terraformer
{
	template<class T, class PixelType>
	concept pixel_replacing_brush = requires(T f, PixelType z, float xi, float eta, float zeta)
	{
		{f(xi, eta, zeta)} -> std::same_as<std::optional<PixelType>>;
	};

	template<class T>
	concept brush_size_modulator = requires(T f, location loc)
	{
		{f(loc)} -> std::same_as<float>;
	};

	template<class PixelType>
	struct default_pixel_replacing_brush
	{
		auto operator()(float x, float y, float z) const
		{
			return x*x + y*y <= 1.0f ? std::optional{z} : std::optional<float>{};
		}
	};

	struct default_brush_size_modulator
	{
		auto operator()(auto...) const
		{
			return 1.0f;
		}
	};

	template<class PixelType,
		pixel_replacing_brush<PixelType> Brush = default_pixel_replacing_brush<PixelType>,
		brush_size_modulator Modulator = default_brush_size_modulator>
	void draw(location loc,
		span_2d<PixelType> target_surface,
		Brush&& brush = Brush{},
		Modulator&& mod = Modulator{})
	{
		auto const h = target_surface.width();
		auto const w = target_surface.height();

		auto const x = loc[0];
		auto const y = loc[1];
		auto const z = loc[2];

		auto const thickness = 0.5f*mod(loc);
		auto const k_min = static_cast<uint32_t>(y - thickness + 0.5f + static_cast<float>(h));
		auto const k_max = static_cast<uint32_t>(y + thickness + 0.5f + static_cast<float>(h));
		auto const l_min = static_cast<uint32_t>(x - thickness + 0.5f + static_cast<float>(w));
		auto const l_max = static_cast<uint32_t>(x + thickness + 0.5f + static_cast<float>(w));

		for(auto k = k_min; k != k_max; ++k)
		{
			for(auto l = l_min; l != l_max; ++l)
			{
				auto const xi = -1.0f
					+ 2.0f*(static_cast<float>(l - l_min) + 0.5f)
						/static_cast<float>(l_max - l_min);

				auto const eta =  - 1.0f
						+ 2.0f*(static_cast<float>(k - k_min) + 0.5f)
						/static_cast<float>(k_max - k_min);

				if(auto val = brush(xi, eta, z); val.has_value())
				{ target_surface(l%w, (h - 1) - k%h ) = *val; }
			}
		}
	}


	template<class PixelType,
		class BrushType = default_pixel_replacing_brush<PixelType>,
		class BrushSizeModulator = default_brush_size_modulator>
	void draw(geosimd::line_segment<geom_space> seg,
		span_2d<PixelType> target_surface,
		BrushType&& brush = BrushType{},
		BrushSizeModulator&& mod = BrushSizeModulator{})
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
				draw(location{x, y, z}, target_surface, brush, mod);
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
				draw(location{x, y, z}, target_surface, brush, mod);
			}
		}
	}

	template<class PixelType,
		class BrushType = default_pixel_replacing_brush<PixelType>,
		class BrushSizeModulator = default_brush_size_modulator>
	void draw_as_line_segments(std::span<location const> curve,
		span_2d<PixelType> target_surface,
		BrushType&& brush = BrushType{},
		BrushSizeModulator&& mod = BrushSizeModulator{})
	{
		if(std::size(curve) == 0)
		{ return; }

		auto prev = curve[0];
		for(size_t k = 1; k!=std::size(curve); ++k)
		{
			auto const current = curve[k];
			draw(geosimd::line_segment{.p1 = prev, .p2 = current}, target_surface, brush, mod);
			prev = current;
		}
	}

	template<class PixelType,
		class BrushType = default_pixel_replacing_brush<PixelType>,
		class BrushSizeModulator = default_brush_size_modulator>
	void draw_as_dots(std::span<location const> curve,
		span_2d<PixelType> target_surface,
		BrushType&& brush = BrushType{},
		BrushSizeModulator&& mod = BrushSizeModulator{})
	{
		for(auto point: curve)
		{
			draw(point, target_surface, brush, mod);
		}
	}

}

#endif