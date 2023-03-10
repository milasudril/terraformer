//@	{"target":{"name":"polynomial.test"}}

#include "./polynomial.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_polynomial_eval)
{
	terraformer::polynomial p{2.0f, 1.0f, -1.0f};

	EXPECT_EQ(p.degree(), 2);

	auto coeffs = p.coefficients();
	EXPECT_EQ(coeffs[0], 2.0f);
	EXPECT_EQ(coeffs[1], 1.0f);
	EXPECT_EQ(coeffs[2], -1.0f);

	auto val_0 = p(0.0f);
	auto val_1 = p(0.5f);
	auto val_2 = p(1.0f);

	EXPECT_EQ(val_0, 2.0f);
	EXPECT_EQ(val_1, 2.25f);
	EXPECT_EQ(val_2, 2.0f);
}
