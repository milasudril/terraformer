#ifndef TERRAFORMER_INTERP_HPP
#define TERRAFORMER_INTERP_HPP

#include <ranges>

namespace terraformer
{
	template<std::ranges::random_access_range R>
	constexpr auto interp(R&& lut, float value)
	{
		if(value < 0.0f)
		{ return lut[0]; }

		if(value >= static_cast<float>(std::size(lut) - 1))
		{ return lut[std::size(lut) - 1]; }

		auto const int_part = static_cast<int>(value);
		auto const t = value - static_cast<float>(int_part);
		auto const left = lut[int_part];
		auto const right = lut[int_part + 1];

		return (1.0f - t)*left + right*t;
	}
}

#endif