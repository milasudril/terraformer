//@	{"target":{"name": "filtered_noise_generator.test"}}

#include "./filtered_noise_generator.hpp"

#include "lib/pixel_store/image.hpp"

#include "lib/pixel_store/image_io.hpp"

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
	terraformer::random_generator rng{};

	terraformer::filtered_noise_generator_2d generator{
		rng,
		terraformer::span_2d_extents{1024, 1024},
		32.0f,
		terraformer::filtered_noise_description_2d{
			.wavelength_x = terraformer::domain_length{8192.0f},
			.wavelength_y = terraformer::domain_length{4096.0f},
			.hp_order = terraformer::filter_order{8.0f},
			.lp_order = terraformer::filter_order{8.0f}
		}
	};

	terraformer::grayscale_image img_out{2048, 2048};
	for(uint32_t y = 0; y != 2048; ++y)
	{
		for(uint32_t x = 0; x != 2048; ++x)
		{
			img_out(x, y) = generator(static_cast<float>(x)*32.0f, static_cast<float>(y)*32.0f);
		}
	}
	store(img_out, "test.exr");
}