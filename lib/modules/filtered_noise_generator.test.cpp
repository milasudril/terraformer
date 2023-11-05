//@	{"target":{"name": "filtered_noise_generator.test"}}

#include "./filtered_noise_generator.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_filtered_noise_generator_generate)
{
	terraformer::random_generator rng{};

	auto res = generate(rng, terraformer::filtered_noise_description{
		.lambda_0 = 14043.0f,
		.hp_order = 8.0f,
		.lp_order = 2.0f,
		.dx = 48,
		.point_count = 1024
	});

	for(size_t k = 0; k != std::size(res); ++k)
	{
		printf("%.8g %.8g\n", (static_cast<float>(k) + 0.5f)*48.0f, res[k]);
	}
}