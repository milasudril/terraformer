#ifndef TERRAFORMER_MATHUTILS_CUBIC_SPLINE_HPP
#define TERRAFORMER_MATHUTILS_CUBIC_SPLINE_HPP

#include "./polynomial.hpp"
#include "lib/common/interval.hpp"
#include "lib/common/bounded_value.hpp"

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

	struct cubic_polynomial_with_crit_point_descriptor
	{
		bounded_value<open_open_interval{0.0f, 1.0f}, 0.5f> x_crit{0.5f};
		float y_0 = 0.0f;
		float y_crit = 0.0f;
		float y_1 = 0.0f;
	};

	constexpr auto make_polynomial(cubic_polynomial_with_crit_point_descriptor const& params)
	{
		auto const y_0 = params.y_0;
		auto const y_crit = params.y_crit;
		auto const y_1 = params.y_1;
		auto const x_crit = params.x_crit;

		// P^\prime(x) = A(x)(x - x_crit)
		// P(x) = y_0 + \int_0^x A(t)(t - x_crit) dt
		// A(t) = at + b
		// P(x) = y_0 + \int_0^x (at + b)(t - x_crit) dt
		//      = y_0 + \int_0^x (at^2 + t(b - a x_crit) - b x_crit) dt
		//      = y_0 + ax^3/3 + (b - a x_crit)x^2/2 - b x_crit x
		//      = y_0 + a(x^3/3 - x_crit x^2/2) + b(x^2/2  - x x_crit)
		//
		// P(x_crit) = y_0 + a(x_crit^3/3 - x_crit^3/2) - b(x_crit^2/2) = y_crit
		// P(1)      = y_0 + a(1/3 - x_crit/2) + b(1/2  - x_crit) = y_1
		//
		auto const x_crit_rev = 1.0f - x_crit;
		auto const det = (x_crit*x_crit_rev)*(x_crit*x_crit_rev);
		auto const a = 3.0f*(x_crit*x_crit*y_1 + y_crit*(1.0f - 2.0f*x_crit) - y_0*x_crit_rev*x_crit_rev)/det;
		auto const b = (
			    y_0*x_crit_rev*x_crit_rev*(2.0f + x_crit)
			  - y_1*x_crit*x_crit*x_crit
			  + y_crit*(3.0f*x_crit - 2.0f)
		)/det;

		return polynomial{y_0, -b*x_crit, (b - a*x_crit)/2.0f, a/3.0f};
	}
}

#endif