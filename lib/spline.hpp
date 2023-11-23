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

	struct spline_control_point
	{
		float y;
		float ddx;
	};

	constexpr auto spline(float x, spline_control_point a,  spline_control_point b)
	{
		auto const y_0 = a.y;
		auto const y_1 = b.y;
		auto const A = a.ddx;
		auto const B = b.ddx;

		// a=B+A-2*y[1]+2*y[0],b=-B-2*A+3*y[1]-3*y[0],c=A,d=y[0]
		auto const cubic = B + A - 2.0*(y_1 - y_0);
		auto const quadratic = -B - 2.0f*A + 3.0f*(y_1-y_0);
		auto const linear = A;
		auto const constant = y_0;

		return cubic*x*x*x + quadratic*x*x + linear*x + constant;
	}

}

#endif