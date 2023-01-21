#ifndef TERRAFORMER_CURVE_RASTERIZER_HPP
#define TERRAFORMER_CURVE_RASTERIZER_HPP

#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"

#include <cstdio>

namespace terraformer
{
	template<class T, class PixelType>
	concept brush = requires(T f, PixelType z, float xi, float eta, float zeta)
	{
		{f(xi, eta, zeta)} -> std::same_as<PixelType>;
	};

	template<class T>
	concept brush_size_modulator = requires(T f, location loc)
	{
		{f(loc)} -> std::same_as<float>;
	};

	template<class PixelType, brush<PixelType> BrushType, brush_size_modulator ModulatorType>
	void draw(location loc, span_2d<PixelType> target_surface, BrushType&& brush, ModulatorType&& mod)
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

				target_surface(l%w, k%h) = brush(xi, eta, z);
			}
		}
	}

	template<class PixelType, brush<PixelType> BrushType, brush_size_modulator ModulatorType>
	void draw(std::span<location const> curve,
		span_2d<PixelType> target_surface,
		BrushType&& brush,
		ModulatorType&& mod)
	{
		for(auto loc : curve)
		{
			draw(loc, target_surface, brush, mod);
		}
	}

}

#endif