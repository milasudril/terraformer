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

	template<class T, class PixelType>
	concept blend_function = requires(T f, PixelType old_val, PixelType new_val, float brush_strength)
	{
		{f(old_val, new_val, brush_strength)} -> std::same_as<PixelType>;
	};

	template<class T, class PixelType>
	concept draw_intensity_modulator = requires(T f, float curve_level, PixelType value)
	{
		{f(curve_level, value)} -> std::same_as<PixelType>;
	};

	struct solid_circle
	{
		constexpr auto operator()(float x, float y) const
		{ return x*x + y*y <= 1.0f ? 1.0f : 0.0f; }
	};

	template<class PixelType>
	struct lerp
	{
		constexpr auto operator()(PixelType old_val, PixelType new_val, float brush_strength) const
		{ return brush_strength*new_val + (1.0f - brush_strength)*old_val; }
	};

	template<class PixelType>
	struct multiply
	{
		constexpr auto operator()(float curve_level, PixelType val) const
		{ return curve_level*val; }
	};

	template<class PixelType,
		brush Brush = solid_circle,
		blend_function<PixelType> BlendFunction = lerp<PixelType>>
	struct paint_params
	{
		float x;
		float y;
		PixelType value;
		float brush_diameter;
		Brush brush{};
		BlendFunction blend_function{};
	};

	template<class PixelType,
		brush Brush,
		blend_function<PixelType> BlendFunction>
	void paint(span_2d<PixelType> target_surface, paint_params<PixelType, Brush, BlendFunction> const& params)
	{
		auto const h = target_surface.width();
		auto const w = target_surface.height();

		auto const value = params.value;
		auto const d = params.brush_diameter;
		auto const thickness = 0.5f*d;
		auto const brush_size = static_cast<uint32_t>(d);
		auto const k_min = static_cast<int32_t>(params.y - thickness + 0.5f);
		auto const l_min = static_cast<int32_t>(params.x - thickness + 0.5f);

		for(auto k = 0u; k != brush_size; ++k)
		{
			for(auto l = 0u; l != brush_size; ++l)
			{
				auto const xi = std::lerp(-1.0f, 1.0f, (static_cast<float>(l) + 0.5f)/d);
				auto const eta = std::lerp(-1.0f, 1.0f, (static_cast<float>(k) + 0.5f)/d);

				auto const strength = params.brush(xi, eta);
				auto const src_val = target_surface((l + l_min + w)%w, (k + k_min + h)%h);
				static_assert(std::is_same_v<decltype(l + l_min), uint32_t>);
				target_surface((l + l_min + w)%w, (k + k_min + h)%h ) =
					params.blend_function(src_val, value, strength);
			}
		}
	}

	template<class T>
	concept brush_size_modulator = requires(T f, float x, float y)
	{
		{f(x, y)} -> std::same_as<float>;
	};

	struct constant_brush_size
	{
		constexpr auto operator()(float, float) const
		{ return value; }

		float value{1.0f};
	};

	template<class PixelType,
		blend_function<PixelType> BlendFunction = lerp<PixelType>,
		draw_intensity_modulator<PixelType> IntensityModulator = multiply<PixelType>,
		brush Brush = solid_circle,
		brush_size_modulator BrushSizeModulator = constant_brush_size>
	struct line_segment_draw_params
	{
		PixelType value;
		BlendFunction blend_function{};
		IntensityModulator intensity_modulator{};
		float scale = 1.0f;
		Brush brush{};
		BrushSizeModulator brush_diameter{};
	};

	template<class ... Args>
	inline auto make_line_segment_draw_params(Args&& ... args)
	{ return line_segment_draw_params<Args...>(std::forward<Args>(args)...); }

	template<class PixelType,
		blend_function<PixelType> BlendFunction = lerp<PixelType>,
		draw_intensity_modulator<PixelType> IntensityModulator = multiply<PixelType>,
		brush Brush,
		brush_size_modulator BrushSizeModulator = constant_brush_size>
	void draw(span_2d<PixelType> target_surface,
		geosimd::line_segment<geom_space> seg,
		line_segment_draw_params<PixelType, BlendFunction, IntensityModulator, Brush, BrushSizeModulator> const& params)
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
				paint(target_surface, paint_params{
					.x = x/params.scale,
		  		.y = y/params.scale,
		  		.value = params.intensity_modulator(z, params.value),
		  		.brush_diameter = params.brush_diameter(x, y),
					.brush = params.brush,
					.blend_function = params.blend_function
				});
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
				paint(target_surface, paint_params{
					.x = x/params.scale,
		  			.y = y/params.scale,
		  			.value = params.intensity_modulator(z, params.value),
		  			.brush_diameter = params.brush_diameter(x, y),
					.brush = params.brush,
					.blend_function =params.blend_function
				});
			}
		}
	}

	template<class PixelType,
		blend_function<PixelType> BlendFunction,
		draw_intensity_modulator<PixelType> IntensityModulator,
		brush Brush,
		brush_size_modulator BrushSizeModulator>
	void draw(span_2d<PixelType> target_surface,
		std::span<location const> curve,
		line_segment_draw_params<PixelType, BlendFunction, IntensityModulator, Brush, BrushSizeModulator> const& params)
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
