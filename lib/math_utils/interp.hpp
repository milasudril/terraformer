#ifndef TERRAFORMER_INTERP_HPP
#define TERRAFORMER_INTERP_HPP

#include "./boundary_sampling_policy.hpp"
#include "lib/common/ranges.hpp"

#include <cmath>

namespace terraformer
{
	template<class T>
	concept has_interp = requires(T left, T right, float t)
	{
		{interp(left, right, t)} -> std::same_as<T>;
	};

	template<class T>
	concept has_lerp = requires(T left, T right, float t)
	{
		{lerp(left, right, t)} -> std::same_as<T>;
	};

	template<std::ranges::random_access_range R, boundary_sampling_policy U>
	constexpr auto interp(R&& lut, float value, U&& bsp)
	{
		auto const n = static_cast<uint32_t>(std::size(lut));
		auto const x = bsp(value, n);
		auto const x_0 = static_cast<uint32_t>(x);;
		auto const x_1 = bsp(x_0 + 1, n);

		auto const left = lut[x_0];
		auto const right = lut[x_1];

		auto const t = x - static_cast<float>(x_0);

		if constexpr(has_interp<std::remove_cvref_t<decltype(left)>>)
		{ return interp(left, right, t); }
		if constexpr(has_lerp<std::remove_cvref_t<decltype(left)>>)
		{ return lerp(left, right, t); }
		else
		{ return (1.0f - t)*left + t*right; }
	}

	template<class T, boundary_sampling_policy U>
	requires(random_access_input_range_2d<T, float>)
	auto interp(T&& image, float x_val, float y_val, U&& bsp)
	{
		auto const w = image.width();
		auto const h = image.height();

		auto const x = bsp(x_val, w);
		auto const y = bsp(y_val, h);

		auto const x_0 = static_cast<uint32_t>(x);
		auto const y_0 = static_cast<uint32_t>(y);
		auto const x_1 = bsp(x_0 + 1, w);
		auto const y_1 = bsp(y_0 + 1, h);

		auto const z_00 = image(x_0, y_0);
		auto const z_01 = image(x_0, y_1);
		auto const z_10 = image(x_1, y_0);
		auto const z_11 = image(x_1, y_1);

		auto const xi = x - static_cast<float>(x_0);
		auto const eta = y  - static_cast<float>(y_0);
		auto const z_x0 = (1.0f - static_cast<float>(xi)) * z_00 + static_cast<float>(xi) * z_10;
		auto const z_x1 = (1.0f - static_cast<float>(xi)) * z_01 + static_cast<float>(xi) * z_11;
		return (1.0f - eta)*z_x0 + eta*z_x1;
	}
}

#endif