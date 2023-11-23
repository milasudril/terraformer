#ifndef TERRAFORMER_SPLINE_HPP
#define TERRAFORMER_SPLINE_HPP

#include <algorithm>

namespace terraformer
{
//	a=-(y-B-A)/4,b=-(A-B)/4,c=(3*y-B-A)/4,d=(2*y-B+A)/4

	constexpr auto smoothstep(float x)
	{
		x = std::clamp(x, -1.0f, 1.0f);
		return (2.0f - x)*(x + 1.0f)*(x + 1.0f)/4.0f;
	}

}

#endif