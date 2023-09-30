//@	{"target":{"name":"utils.test"}}

#include "./utils.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_mod)
{
	std::array<float, 16> vals{
		0.0f,
		0.25f,
		0.5f,
		0.75f,
		1.0f,
		1.25f,
		1.5f,
		1.75f,
		2.0f,
		2.25f,
		2.5f,
		2.75f,
		3.0f,
		3.25f,
		3.5f,
		3.75f,
	};

	for(size_t k = 0; k != 65; ++k)
	{
		auto const x = 0.25f*(static_cast<float>(k) - 32);
		EXPECT_EQ(terraformer::mod(x, 4.0f), vals[k%16]);
   }
}

TESTCASE(terraformer_approx_sine)
{
	auto const dx = 2.0f*std::numbers::pi_v<float>/12.0f;
	for(int k = -24; k != 24; ++k)
	{
		auto const x = dx*static_cast<float>(k);
		printf("%.8g %.8g\n", x, terraformer::approx_sine(x));
	}
}

TESTCASE(terraformer_smoothstep)
{
	auto const dx = 1.0f/4.0f;
	for(int k = -16; k <= 16; ++k)
	{
		auto const x = dx*static_cast<float>(k);
		printf("%.8g   %.8g\n", x, terraformer::smoothstep(x));
	}
}

TESTCASE(terraformer_round_to_n_digits)
{
	{
		auto res = terraformer::round_to_n_digits(512.0f, 1);
		EXPECT_EQ(res, 512.0f);
	}

	{
		auto res = terraformer::round_to_n_digits(900.0f, 1);
		EXPECT_EQ(res, 1024.0f);
	}
}