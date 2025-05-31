#ifndef TERRAFORMER_MATHUTILS_QUINTIC_POLYNOMIAL_HPP
#define TERRAFORMER_MATHUTILS_QUINTIC_POLYNOMIAL_HPP

#include "./polynomial.hpp"
#include "lib/common/interval.hpp"
#include "lib/common/bounded_value.hpp"

namespace terraformer
{
	struct quintic_polynomial_descriptor
	{
		bounded_value<open_open_interval{0.0f, 1.0f}, 0.5f> x_m{0.5f};

		float y_0 = 0.0f;
		float y_m = 0.0f;
		float y_1 = 0.0f;
		float ddx_0 = 0.0f;
		float ddx_m = 0.0f;
		float ddx_1 = 0.0f;
	};

	constexpr auto make_polynomial(quintic_polynomial_descriptor const& p)
	{
		// Decompose the polynomial in two
		//
		// * Value controlling polynomial V(x), with is derivative equal to zero at all control points
		// * Derivative controlling polynomial D(x), with its value equal to zero at all control points
		//
		// The final polynomial P(x) = V(x) + D(x) will satisfy the design contraints. Notice that both
		// D(x) and V^\prime(x) will contain the factor x(x - x_m)(x - 1). Since it describes known
		// zeros, call it Z. Notice that
		//
		// Z^\prime(x) = 3x^2 - 2x - 2x_m*x + x_m = 3x^2 - 2x(1 + x_m) + x_m
		//
		//
		// The derivative controlling polynomial
		//
		// D(x) = A(x)Z(x) => D^\prime(x) = A(x)Z^\prime(x) + A^\prime(x)Z(x)
		//
		// D\^prime(0) = A(0)Z^\prime(0) + A^\prime(0)Z(0) = A(0)x_m = ddx_0
		// D\^prime(x_m) = A(x_m)Z^\prime(x_m) + A^\prime(x_m)Z(x_m) = A(x_m)x_m(x_m - 1) = ddx_m
		// D\^prime(1) = A(1)Z^\prime(1) + A^\prime(1)Z(1) = A(1)(1 - x_m) = ddx_1
		//
		// Let A(X) = a_2x^2 + a_1x + a_0
		//
		// Then
		//
		// a_0 x_m = ddx_0
		// (a_2 x_m^2 + a_1 x_m + a_0)x_m(x_m - 1) = ddx_m
		// (a_2 + a_1 + a_0)(1 - x_m) = ddx_1
		//
		auto const a_0 = p.ddx_0/p.x_m;
		auto const x_m_rev = 1.0f - p.x_m;
		auto const a_denom = (x_m_rev*x_m_rev)*(p.x_m*p.x_m);
		auto const a_1 = (
			- p.ddx_m
			- p.ddx_0
			- p.ddx_1*(p.x_m*p.x_m*p.x_m)
			+ p.ddx_0*p.x_m
			+ p.ddx_0*(p.x_m*p.x_m)
			- p.ddx_0*(p.x_m*p.x_m*p.x_m)
		)/a_denom;
		auto const a_2 = (
			  p.ddx_0
			+ p.ddx_m
			- 2.0f*p.ddx_0*p.x_m
			+ p.ddx_1*(p.x_m*p.x_m)
			+ p.ddx_0*(p.x_m*p.x_m)
		)/a_denom;

		auto const Z = polynomial{0.0f, 1.0f}*polynomial{-p.x_m, 1.0f}*polynomial{-1.0f, 1.0f};
		static_assert(decltype(Z)::degree() == 3);
		auto const A = polynomial{a_0, a_1, a_2};
		static_assert(decltype(A)::degree() == 2);
		auto const D = A*Z;
		static_assert(decltype(D)::degree() == 5);


		// The value controlling polynomial
		//
		// V^\prime(x) = B(x)Z(x) => V(x) = y_0 + \int_0^x B(t)Z(t) dt
		//
		// Let B(t) = b_1 t + b_0. After computing the integral
		//
		// V(x) = y_0 + p_2 x^2 + p_3 x^3 + p_4 x^4 + p_5 x^5,
		//
		// where
		//
		// p_5 = b_1/5
    // p_4 = (b_0 - b_1(x_m + 1))/4
    // p_3 = (b_1 x_m - b_0(x_m + 1))/3
    // p_2 = (b_0 x_m)/2
		//
		// The parameters b_0 and b_1 are found by letting V(x_m) = y_m, and V(1) = y_1.
		//
		// The result is, with det = (x_m(1 - x_m))^3
		//
    // b_1 = 10((y_m - y_0)*(2*x_m - 1) - (x_m^3)(2 - x_m)(y_1 - y_0))/det
    // b_0 = 2( (x_m^4)(5 - 3*x_m)(y_1 - y_0) - (5x_m - 3)(y_m - y_0))/det
		//
		auto const det = (p.x_m*p.x_m*p.x_m)*(x_m_rev*x_m_rev*x_m_rev);
		auto const b_1 =
			10.0f*((p.y_m - p.y_0)*(2.0f*p.x_m - 1.0f) - (p.x_m*p.x_m*p.x_m)*(2.0f - p.x_m)*(p.y_1 - p.y_0))
			/det;
		auto const b_0 =
			2.0f*(
				(p.x_m*p.x_m)*(p.x_m*p.x_m)*(5.0f - 3.0f*p.x_m)*(p.y_1 - p.y_0) - (5.0f*p.x_m - 3.0f)*(p.y_m - p.y_0)
			)/det;

		auto const p_5 = b_1/5.0f;
    auto const p_4 = (b_0 - b_1*(p.x_m + 1.0f))/4.0f;
    auto const p_3 = (b_1*p.x_m - b_0*(p.x_m + 1.0f))/3.0f;
    auto const p_2 = (b_0*p.x_m)/2.0f;

    auto const V = polynomial{p.y_0, 0.0f, p_2, p_3, p_4, p_5};
		static_assert(decltype(V)::degree() == 5);

		return V + D;
	}
}

#endif