//@	{"target":{"name":"ridge_curve.test"}}

#include "./ridge_curve.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ridge_curve_generate)
{
	terraformer::ridge_curve_description curve{
		.amplitude = terraformer::horizontal_amplitude{3096.0f},
		.wavelength = terraformer::domain_length{12384.0f},
		.flip_direction = false,
		.invert_displacement = false
	};
	terraformer::random_generator rng;
	auto const res = generate(curve, rng, 49152, 1.0f);
	EXPECT_EQ(std::size(res), 49152);
	for(size_t k = 0; k != std::size(res); ++k)
	{
		printf("%.8g %.8g\n", res[k][0], res[k][1]);
	}
}