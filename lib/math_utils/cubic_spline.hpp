#ifndef TERRAFORMER_MATHUTILS_CUBIC_SPLINE_HPP
#define TERRAFORMER_MATHUTILS_CUBIC_SPLINE_HPP

#include "./polynomial.hpp"
#include "./interp.hpp"

#include "lib/array_classes/single_array.hpp"
#include "lib/array_classes/multi_array.hpp"

#include <algorithm>

namespace terraformer
{
	template<class Point, class Vector>
	struct cubic_spline_control_point
	{
		Point y;
		Vector ddx;

		constexpr bool operator==(cubic_spline_control_point const&) const = default;
		constexpr bool operator!=(cubic_spline_control_point const&) const = default;
	};

	template<class Point, class Vector>
	constexpr auto make_polynomial(
		cubic_spline_control_point<Point, Vector> a,
		cubic_spline_control_point<Point, Vector> b
	)
	{
		auto const y_0 = a.y;
		auto const y_1 = b.y;
		auto const A = a.ddx;
		auto const B = b.ddx;

		// a=B + A - 2*y[1] + 2*y[0], b = -B -2*A + 3*y[1] - 3*y[0], c=A, d=y[0]
		auto const cubic = B + A - 2.0f*(y_1 - y_0);
		auto const quadratic = -B - 2.0f*A + 3.0f*(y_1 - y_0);
		auto const linear = A;
		auto const constant = y_0 - Point{};

		return polynomial{constant, linear, quadratic, cubic};
	}

	template<class Point, class Vector>
	constexpr auto interp(
		cubic_spline_control_point<Point, Vector> a,
		cubic_spline_control_point<Point, Vector> b,
		float x
	)
	{	return Point{} + make_polynomial(a, b)(x); }

	constexpr auto smoothstep(float x)
	{
		x = std::clamp(x, -1.0f, 1.0f);
		return (2.0f - x)*(x + 1.0f)*(x + 1.0f)/4.0f;
	}

	template<class Point, class Vector>
	auto make_point_array(
		cubic_spline_control_point<Point, Vector> begin,
		cubic_spline_control_point<Point, Vector>  end,
		size_t count
	)
	{ return make_point_array<Point>(make_polynomial(begin, end), count); }
}

#endif