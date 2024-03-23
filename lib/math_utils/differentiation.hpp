#ifndef TERRAFORMER_DIFFERENTIATION_HPP
#define TERRAFORMER_DIFFERENTIATION_HPP

#include "./interp.hpp"

#include "lib/common/ranges.hpp"

namespace terraformer
{
	template<class R, boundary_sampling_policy U>
	requires random_access_input_range_2d<R, float>
	inline auto grad(R&& range, float x, float y, float scale, U&& bsp)
	{
		auto const x0 = x - 1.0f;
		auto const x1 = x + 1.0f;
		auto const y0 = y - 1.0f;
		auto const y1 = y + 1.0f;

		auto const z_x1_y = interp(range, x1, y, bsp);
		auto const z_x0_y = interp(range, x0, y, bsp);
		auto const z_x_y1 = interp(range, x, y1, bsp);
		auto const z_x_y0 = interp(range, x, y0, bsp);

		return 0.5f*scale*displacement{z_x1_y - z_x0_y, z_x_y1 - z_x_y0, 0.0f};
	}

	template<class R, boundary_sampling_policy U>
	requires random_access_input_range_2d<R, float>
	inline auto grad(R&& range, uint32_t x, uint32_t y, float scale, U&& bsp)
	{
		auto const w = range.width();
		auto const h = range.height();

		auto const x0 = bsp(static_cast<int32_t>(x) - 1, w);
		auto const x1 = bsp(static_cast<int32_t>(x) + 1, w);
		auto const y0 = bsp(static_cast<int32_t>(y) - 1, h);
		auto const y1 = bsp(static_cast<int32_t>(y) + 1, h);

		auto const z_x1_y = range(x1, y);
		auto const z_x0_y = range(x0, y);
		auto const z_x_y1 = range(x, y1);
		auto const z_x_y0 = range(x, y0);

		return 0.5f*scale*displacement{z_x1_y - z_x0_y, z_x_y1 - z_x_y0, 0.0f};
	}

	template<class R, boundary_sampling_policy U>
	requires random_access_input_range_2d<R, float>
	inline auto normal(R&& range, uint32_t x, uint32_t y, float scale, U&& bsp)
	{
		auto const g = grad(range, x, y, scale, bsp);
		return direction{displacement{-g[0], -g[1], 1.0f}};
	}
}
#endif