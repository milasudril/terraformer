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
		{interp(left, right, t)};
	};

	template<class T>
	concept has_lerp = requires(T left, T right, float t)
	{
		{lerp(left, right, t)};
	};

	template<class T>
	concept has_index_type = requires
	{
		typename T::index_type;
	};

	template<class T>
	struct select_index_type
	{
		using type = size_t;
	};

	template<class T>
	requires has_index_type<T>
	struct select_index_type<T>
	{
		using type = typename T::index_type;
	};

	template<class T>
	auto make_interpolator(T&& a, T&& b)
	{
		if constexpr(has_interp<std::remove_cvref_t<decltype(a)>>)
		{
			return [a = std::forward<T>(a), b = std::forward<T>(b)](float t){
				return interp(a, b, t);
			};
		}
		else
		if constexpr(has_lerp<std::remove_cvref_t<decltype(a)>>)
		{
			return [a = std::forward<T>(a), b = std::forward<T>(b)](float t){
				return lerp(a, b, t);
			};
		}
		else
		{
			return [a = std::forward<T>(a), b = std::forward<T>(b)](float t){
				return (1.0f - t)*a + t*b;
			};
		}
	}

	template<std::ranges::random_access_range R, boundary_sampling_policy U>
	constexpr auto interp(R&& lut, float value, U&& bsp)
	{
		using array_type = std::remove_cvref_t<R>;

		using index_type = typename select_index_type<array_type>::type;

		// HACK: Need a way to find the representation of std::size(lut)
		using size_rep = size_t;

		// HACK: Do not force conversion to uint32_t. bsp must accept any integral type
		auto const n = static_cast<uint32_t>(static_cast<size_rep>(std::size(lut)));
		auto const x = bsp(value, n);
		auto const x_0 = static_cast<uint32_t>(x);
		auto const x_1 = static_cast<index_type>(bsp(x_0 + 1, n));

		auto const left = lut[static_cast<index_type>(x_0)];
		auto const right = lut[x_1];

		auto const t = x - static_cast<float>(x_0);

		if constexpr(has_interp<std::remove_cvref_t<decltype(left)>>)
		{ return interp(left, right, t); }
		else
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