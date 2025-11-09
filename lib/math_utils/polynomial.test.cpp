//@	{"target":{"name":"polynomial.test"}}

#include "./polynomial.hpp"

#include "lib/common/spaces.hpp"
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

TESTCASE(terraformer_polynomial_add)
{
	terraformer::polynomial const p1{
		1.0f,
		2.0f,
		3.0f
	};

	terraformer::polynomial const p2{
		1.0f,
		2.0f,
		3.0f,
		4.0f
	};

	auto const p3 = p1 + p2;
	EXPECT_EQ(p3.degree(), 3);
	EXPECT_EQ(p3.coefficients[0], 2.0f);
	EXPECT_EQ(p3.coefficients[1], 4.0f);
	EXPECT_EQ(p3.coefficients[2], 6.0f);
	EXPECT_EQ(p3.coefficients[3], 4.0f);

	auto const p3_other = p2 + p1;
	EXPECT_EQ(p3_other, p3);
}

TESTCASE(terraformer_polynomial_multiply)
{
	terraformer::polynomial const p1{
		1.0f,
		1.0f
	};

	terraformer::polynomial const p2{
		1.0f,
		1.0f
	};

	auto const p3 = p1*p2;
	EXPECT_EQ(p3.degree(), 2);
	EXPECT_EQ(p3.coefficients[0], 1.0f);
	EXPECT_EQ(p3.coefficients[1], 2.0f);
	EXPECT_EQ(p3.coefficients[2], 1.0f);

	auto const p4 = p1*p1*p1;
	EXPECT_EQ(p4.degree(), 3);
	EXPECT_EQ(p4.coefficients[0], 1.0f);
	EXPECT_EQ(p4.coefficients[1], 3.0f);
	EXPECT_EQ(p4.coefficients[2], 3.0f);
	EXPECT_EQ(p4.coefficients[3], 1.0f);
}

TESTCASE(terraformer_polynomial_multiply_vectors_coeffs)
{
	terraformer::polynomial const p1{
		terraformer::displacement{1.0f, 2.0f, 3.0f},
		terraformer::displacement{2.0f, 3.0f, 1.0f}
	};

	terraformer::polynomial const p2{
		terraformer::displacement{1.0f, 2.0f, 3.0f},
		terraformer::displacement{2.0f, 3.0f, 1.0f}
	};

	auto const p3 = multiply(p1, p2, [](auto a, auto b){
		return inner_product(a, b);
	});

	EXPECT_EQ(p3.degree(), 2);
	EXPECT_EQ(p3.coefficients[0], 14.0f);
	EXPECT_EQ(p3.coefficients[1], 22.0f);
	EXPECT_EQ(p3.coefficients[2], 14.0f);
}