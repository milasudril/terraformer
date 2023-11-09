#ifndef TERRAFORMER_INTERP_HPP
#define TERRAFORMER_INTERP_HPP

#include "./boundary_sampling_policy.hpp"

#include <ranges>

namespace terraformer
{
	template<class T, class U>
	concept readable_image = requires(T image, uint32_t x, uint32_t y)
	{
		{image.width()} -> std::same_as<uint32_t>;
		{image.height()} -> std::same_as<uint32_t>;
		{image(x, y)} -> std::convertible_to<U const&>;
	};

	template<std::ranges::random_access_range R, boundary_sampling_policy U>
	constexpr auto interp(R&& lut, float value, U&& bsp)
	{
		auto const n = static_cast<uint32_t>(std::size(lut));
		auto const x = bsp(value, n);
		auto const x_0 = static_cast<uint32_t>(x);;
		auto const x_1 = bsp(x_0 + 1, n);

		auto const t = x - static_cast<float>(x_0);
		auto const left = lut[x_0];
		auto const right = lut[x_1];

		return (1.0f - t)*left + right*t;
	}
}

#endif