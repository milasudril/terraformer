#ifndef TERRAFORMER_TRIGFUNC_HPP
#define TERRAFORMER_TRIGFUNC_HPP

#include "./mod.hpp"

#include <numbers>

namespace terraformer
{
	constexpr auto approx_sine(float x)
	{
		auto const xi = 2.0f*(0.5f - mod(x/(2.0f*std::numbers::pi_v<float>), 1.0f));
		return -(xi*(xi - 1.0f)*(xi + 1.0f)
			*((13968.0f/31680.0f)*(xi*xi)*(xi*xi) - (63304.0f/31680.0f)*(xi*xi) + 99433.0f/31680.0f));
	}
}

#endif