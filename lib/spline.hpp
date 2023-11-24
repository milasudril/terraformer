#ifndef TERRAFORMER_SPLINE_HPP
#define TERRAFORMER_SPLINE_HPP

#include <algorithm>

namespace terraformer
{
	constexpr auto smoothstep(float x)
	{
		x = std::clamp(x, -1.0f, 1.0f);
		return (2.0f - x)*(x + 1.0f)*(x + 1.0f)/4.0f;
	}
}

#endif