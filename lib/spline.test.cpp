//@	{"target":{"name":"utils.test"}}

#include "./spline.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_smoothstep)
{
	auto const dx = 1.0f/4.0f;
	for(int k = -16; k <= 16; ++k)
	{
		auto const x = dx*static_cast<float>(k);
		printf("%.8g   %.8g\n", x, terraformer::smoothstep(x));
	}
}