#ifndef TERRAFORMER_BOUNDARY_SAMPLING_POLISIES_HPP
#define TERRAFORMER_BOUNDARY_SAMPLING_POLISIES_HPP

#include "./mod.hpp"

#include <algorithm>

namespace terraformer
{
	struct wrap_around_at_boundary
	{
		auto operator()(float x, uint32_t max) const
		{ return static_cast<uint32_t>(mod(x, static_cast<float>(max))); }

		auto operator()(int32_t x, uint32_t max) const
		{	return static_cast<uint32_t>(mod(x, static_cast<int32_t>(max))); }

		auto operator()(uint32_t x, uint32_t max) const
		{ return x % max; }
	};

	struct clamp_at_boundary
	{
		uint32_t operator()(float x, uint32_t max) const
		{ return static_cast<uint32_t>(std::clamp(x, 0.0f, static_cast<float>(max - 1))); }

		uint32_t operator()(int32_t x, uint32_t max) const
		{ return static_cast<uint32_t>(std::clamp(x, 0, static_cast<int32_t>(max) - 1)); }

		uint32_t operator()(uint32_t x, uint32_t max) const
		{ return std::clamp(x, 0u, max - 1); }
	};
}

#endif