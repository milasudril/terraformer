//@	{"target":{"name": "quintic_polynomial.test"}}

#include "./quintic_polynomial.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_quintic_polynomial_function_values_with_zero_derivatives)
{
	auto const p = make_polynomial(
		terraformer::quintic_polynomial_descriptor{
			.x_m = 0.75f,
			.y_0 = 1.0f,
			.y_m = 2.0f,
			.y_1 = 1.5f,
			.ddx_0 = 0.0f,
			.ddx_m = 0.0f,
			.ddx_1 = 0.0f
		}
	);

	auto const y_0 = p(0.0f);
	auto const y_m = p(0.75f);
	auto const y_1 = p(1.0f);

	EXPECT_EQ(y_0, 1.0f);
	EXPECT_EQ(y_m, 2.0f);
	EXPECT_LT(std::abs(y_1 - 1.5f), 1.0e-4f);

	auto const dp = p.derivative();
	static_assert(decltype(dp)::degree() == decltype(p)::degree() - 1);
	EXPECT_EQ(dp(0.0f), 0.0f);
	EXPECT_LT(std::abs(dp(0.75f)), 1.0e-4f);
	EXPECT_LT(std::abs(dp(1.0f)), 1.0e-4f);
}

TESTCASE(terraformer_quintic_polynomial_derivatives_with_zero_values)
{
	auto const p = make_polynomial(
		terraformer::quintic_polynomial_descriptor{
			.x_m = 0.75f,
			.y_0 = 0.0f,
			.y_m = 0.0f,
			.y_1 = 0.0f,
			.ddx_0 = 1.0f,
			.ddx_m = 2.0f,
			.ddx_1 = 3.0f
		}
	);

	EXPECT_EQ(p(0.0f), 0.0f);
	EXPECT_EQ(p(0.75f), 0.0f);
	EXPECT_EQ(p(1.0f), 0.0f);

	auto const dp = p.derivative();
	static_assert(decltype(dp)::degree() == decltype(p)::degree() - 1);
	EXPECT_EQ(dp(0.0f), 1.0f);
	EXPECT_LT(std::abs(dp(0.75f) - 2.0f), 1.0e-4f);
	EXPECT_LT(std::abs(dp(1.0f) - 3.0f), 1.0e-4f);
}

TESTCASE(terraformer_quintic_polynomial_non_zero_values_and_derivatives)
{
	auto const p = make_polynomial(
		terraformer::quintic_polynomial_descriptor{
			.x_m = 0.75f,
			.y_0 = 1.0f,
			.y_m = 2.0f,
			.y_1 = 1.5f,
			.ddx_0 = 1.0f,
			.ddx_m = 2.0f,
			.ddx_1 = 3.0f
		}
	);

	EXPECT_EQ(p(0.0f), 1.0f);
	EXPECT_LT(std::abs(p(0.75f) - 2.0f), 1.0e-4f);
	EXPECT_EQ(p(1.0f), 1.5f);

	auto const dp = p.derivative();
	static_assert(decltype(dp)::degree() == decltype(p)::degree() - 1);
	EXPECT_EQ(dp(0.0f), 1.0f);
	EXPECT_LT(std::abs(dp(0.75f) - 2.0f), 1.0e-4f);
	EXPECT_LT(std::abs(dp(1.0f) - 3.0f), 1.0e-3f);
}