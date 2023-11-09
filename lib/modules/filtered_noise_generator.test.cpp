//@	{"target":{"name": "filtered_noise_generator.test"}}

#include "./filtered_noise_generator.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_filtered_noise_generator_generate)
{
	terraformer::random_generator rng{};

	terraformer::filtered_noise_generator_1d generator{
		rng,
		1024,
		48.0f,
		terraformer::filtered_noise_description_1d{
			.lambda_0 = 14043.0f,
			.hp_order = 8.0f,
			.lp_order = 2.0f,
			.peak_offset = 6144.0f,
		}
	};

	EXPECT_EQ(generator.dx(), 48.0f);

	for(size_t k = 0; k != 4096; ++k)
	{
		auto const x = (static_cast<float>(k) + 0.5f)*generator.dx();
		auto const y = generator(x);
		printf("%zu %.8g\n", k, y);
	}
}