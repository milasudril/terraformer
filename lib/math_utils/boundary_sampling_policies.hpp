#ifndef TERRAFORMER_BOUNDARY_SAMPLING_POLICIES_HPP
#define TERRAFORMER_BOUNDARY_SAMPLING_POLICIES_HPP

#include "lib/math_utils/mod.hpp"

#include <algorithm>

namespace terraformer
{
	struct wrap_around_at_boundary
	{
		constexpr auto operator()(float x, uint32_t max) const
		{ return mod(x, static_cast<float>(max)); }

		constexpr auto operator()(int32_t x, uint32_t max) const
		{	return static_cast<uint32_t>(mod(x, static_cast<int32_t>(max))); }

		constexpr auto operator()(uint32_t x, uint32_t max) const
		{ return x % max; }
	};

	struct clamp_at_boundary
	{
		constexpr auto operator()(float x, uint32_t max) const
		{ return std::clamp(x, 0.0f, static_cast<float>(max - 1)); }

		constexpr auto operator()(int32_t x, uint32_t max) const
		{ return static_cast<uint32_t>(std::clamp(x, 0, static_cast<int32_t>(max) - 1)); }

		constexpr auto operator()(uint32_t x, uint32_t max) const
		{ return std::clamp(x, 0u, max - 1); }
	};
}

#endif