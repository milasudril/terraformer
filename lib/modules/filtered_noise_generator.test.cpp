//@	{"target":{"name": "filtered_noise_generator.test"}}

#include "./filtered_noise_generator.hpp"

#include "lib/pixel_store/image.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_filtered_noise_generator_generate)
{
	terraformer::random_generator rng{};

	terraformer::filtered_noise_generator_1d generator{
		rng,
		1024,
		32.0f,
		terraformer::filtered_noise_description_1d{
			.wavelength = terraformer::domain_length{8192.0f},
			.hp_order = terraformer::filter_order{8.0f},
			.lp_order = terraformer::filter_order{8.0f}
		}
	};

	EXPECT_EQ(generator.dx(), 32.0f);
	EXPECT_EQ(generator(0.0f), 1.0f);
	EXPECT_EQ(generator(65536.0f), 1.0f);

	auto y_0 = generator(0.0f);
	auto goes_up = false;
	for(size_t k = 1; k != 2048; ++k)
	{
		auto const x = static_cast<float>(k)*32.0f;
		auto const y = generator(x);
		if(goes_up)
		{
			if(y < y_0)
			{
				EXPECT_GT(x, 7680.0f);
				EXPECT_LT(x, 8704.0f);
				break;
			}
		}
		else
		{
			if(y > y_0)
			{ goes_up = true; }
		}
		y_0 = y;
	}
}

TESTCASE(terraformer_filtered_noise_generator_2d_generate)
{
#if 0
	span_2d<float const> input, span_2d<float> output, double lambda_max, filtered_noise_description_2d const& params
#endif

	terraformer::grayscale_image dummy{128, 128};
	terraformer::random_generator rng{};
	std::uniform_real_distribution U{0.0f, 1.0f};
	for(uint32_t y = 0; y != dummy.height(); ++y)
	{
		for(uint32_t x = 0; x != dummy.width(); ++x)
		{
			dummy(x, y) = U(rng);
		}
	}

	apply_filter(
		dummy,
		dummy,
		2.0*128*512.0,
		terraformer::filtered_noise_description_2d{
			.wavelength_x = terraformer::domain_length{8192.0f},
			.wavelength_y = terraformer::domain_length{8192.0f},
			.hp_order = terraformer::filter_order{1.0f},
			.lp_order = terraformer::filter_order{2.0f}
		}
	);
}