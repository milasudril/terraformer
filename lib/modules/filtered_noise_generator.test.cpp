//@	{"target":{"name": "filtered_noise_generator.test"}}

#include "./filtered_noise_generator.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/rng.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_filtered_noise_1d_generator_apply_filter_result_is_real)
{
	std::array<float, 1024> input;
	std::generate_n(
		std::data(input),
		std::size(input),
		[
			rng = terraformer::random_generator{},
			U = std::uniform_real_distribution{0.0f, 1.0f}
		]() mutable {
			return U(rng);
		}
	);

	std::array<std::complex<double>, 1024> output;
	apply_filter(
		input,
		output,
		1024.0,
		terraformer::filtered_noise_1d_description{
			.wavelength = terraformer::domain_length{256.0f},
			.hp_order = terraformer::filter_order{2.0f},
			.lp_order = terraformer::filter_order{2.0f}
		}
	);

	for(size_t k = 0; k != std::size(output); ++k)
	{ EXPECT_LT(std::abs(output[k].imag()/output[k].real()), 1e-7f); }
}

TESTCASE(terraformer_filtered_noise_generator_generate)
{
	terraformer::random_generator rng{};

	terraformer::filtered_noise_1d_generator generator{
		rng,
		1024,
		32.0f,
		terraformer::filtered_noise_1d_description{
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

TESTCASE(terraformer_filtered_noise_generator_2d_apply_filter_result_is_real)
{
	terraformer::grayscale_image noise{1024, 1024};
	generate(
		noise.pixels(),
		[
			rng = terraformer::random_generator{},
			U = std::uniform_real_distribution{0.0f, 1.0f}
		](auto...) mutable {
			return U(rng);
		}
	);

	terraformer::basic_image<std::complex<double>> output{1024, 1024};
	apply_filter(
		noise.pixels(),
		output.pixels(),
		1024.0,
		terraformer::filtered_noise_2d_description{
			.wavelength_x = terraformer::domain_length{256.0f},
			.wavelength_y = terraformer::domain_length{256.0f},
			.hp_order = terraformer::filter_order{2.0f},
			.lp_order = terraformer::filter_order{2.0f}
		}
	);

	for(uint32_t y = 0; y != 1024; ++y)
	{
		for(uint32_t x = 0; x != 1024; ++x)
		{ EXPECT_LT(std::abs(output(x, y).imag()/output(x, y).real()), 1e-7f); }
	}
}

#if 0
// TODO: Validate number of periods in rows/colums and the total number of local
//       maxima
TESTCASE(terraformer_filtered_noise_generator_2d_generate_period_count)
{
	terraformer::random_generator rng{};

	terraformer::filtered_noise_generator_2d generator{
		rng,
		terraformer::span_2d_extents{1024, 1024},
		32.0f,
		terraformer::filtered_noise_2d_description{
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
#endif