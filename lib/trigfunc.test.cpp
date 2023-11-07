//@	{"target":{"name":"trigfunc.test"}}

#include "./trigfunc.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_approx_sine)
{
	auto const dx = 2.0f*std::numbers::pi_v<float>/12.0f;
	for(int k = -24; k != 24; ++k)
	{
		auto const x = dx*static_cast<float>(k);
		printf("%.8g %.8g\n", x, terraformer::approx_sine(x));
	}
}
