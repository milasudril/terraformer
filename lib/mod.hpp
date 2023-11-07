#ifndef TERRAFORMER_MOD_HPP
#define TERRAFORMER_MOD_HPP

#include <cstdint>

namespace terraformer
{
	constexpr auto fast_fmod(float val, float divisor)
	{ return val - divisor * static_cast<int>(val / divisor); }

	constexpr auto mod(float x, float denom)
	{
		auto const ret = fast_fmod(x, denom);
		auto const alt_ret = ret + denom;
		return ret < 0.0f ? (alt_ret > 0.0f ? alt_ret : 0.0f) : ret;
	}

	constexpr auto mod(int32_t x, int32_t denom)
	{ return ((x % denom) + denom) % denom; }
}

#endif