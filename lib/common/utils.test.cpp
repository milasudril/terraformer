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
		EXPECT_EQ(terraformer::mod(x, 4), vals[k%16]);
    }
}