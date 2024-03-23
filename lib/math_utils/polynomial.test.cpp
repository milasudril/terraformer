//@	{"target":{"name":"polynomial.test"}}

#include "./polynomial.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_polynomial_degree_value_derivative)
{
	terraformer::polynomial const p{
		1.0f,
		2.0f,
		3.0f
	};

	EXPECT_EQ(p.degree(), 2);
	EXPECT_EQ(p(2.0f), 1.0f + 2.0f*2.0f + 3.0f*4.0f);

	auto const derivative = p.derivative();
	EXPECT_EQ(derivative.degree(), 1);
	EXPECT_EQ(derivative.coefficients[0], 2.0f);
	EXPECT_EQ(derivative.coefficients[1], 3.0f*2.0f);
}