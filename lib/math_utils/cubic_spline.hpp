#ifndef TERRAFORMER_MATHUTILS_CUBIC_SPLINE_HPP
#define TERRAFORMER_MATHUTILS_CUBIC_SPLINE_HPP

#include "./polynomial.hpp"

#include <algorithm>

namespace terraformer
{
	struct cubic_spline_control_point
	{
		float y;
		float ddx;
	};

	constexpr auto make_polynomial(cubic_spline_control_point a, cubic_spline_control_point b)
	{
		auto const y_0 = a.y;
		auto const y_1 = b.y;
		auto const A = a.ddx;
		auto const B = b.ddx;

		// a=B+A-2*y[1]+2*y[0],b=-B-2*A+3*y[1]-3*y[0],c=A,d=y[0]
		auto const cubic = B + A - 2.0f*(y_1 - y_0);
		auto const quadratic = -B - 2.0f*A + 3.0f*(y_1-y_0);
		auto const linear = A;
		auto const constant = y_0;

		return polynomial{constant, linear, quadratic, cubic};
	}

	constexpr auto interp(cubic_spline_control_point a, cubic_spline_control_point b, float x)
	{	return make_polynomial(a, b)(x); }

	constexpr auto smoothstep(float x)
	{
		x = std::clamp(x, -1.0f, 1.0f);
		return (2.0f - x)*(x + 1.0f)*(x + 1.0f)/4.0f;
	}
}

#endif