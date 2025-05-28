//@	{"target": {"name": "./rolling_hills_generator.o"}}

#include "./rolling_hills_generator.hpp"

#include "lib/pixel_store/image_io.hpp"

namespace
{
	terraformer::grayscale_image make_filter(uint32_t width, uint32_t height, float f_x, float f_y)
	{
		printf("f_x = %.8g f_y = %.8g\n", f_x, f_y);
		f_x *= 2.0f*std::numbers::pi_v<float>;
		f_y *= 2.0f*std::numbers::pi_v<float>;

		auto const w_float = static_cast<float>(width);
		auto const h_float = static_cast<float>(height);
		auto const x_0 = 0.5f*(w_float - 1.0f);
		auto const x_y = 0.5f*(h_float - 1.0f);
		terraformer::grayscale_image ret{width, height};
		for(uint32_t y = 0; y != height; ++y)
		{
			for(uint32_t x = 0; x != width; ++x)
			{
				auto const xi = (static_cast<float>(x) - x_0)/f_x;
				auto const eta = (static_cast<float>(y) - x_y)/f_y;

				auto const r2 = xi*xi + eta*eta;
				auto const r4 = r2*r2;

				ret(x, y) = 2.0f*(1.0f/std::sqrt(1.0f + r4))*(r2/std::sqrt(r4 + 1.0f));
			}
		}

		return ret;
	}
}

terraformer::grayscale_image
terraformer::generate(domain_size_descriptor const& size, rolling_hills_descriptor const& params)
{
	auto const normalized_f_x = size.width/params.wavelength_x;
	auto const normalized_f_y = size.height/params.wavelength_y;
	// Assume a bandwidth of at most 6 octaves = 64 periods
	// Take 4 samples per period
	// This gives a size of 256 pixels, but the size is multiplied by 2 to guarantee an even number.
	// Therefore, use 128 pixels as factor
	auto const min_pixel_count = 128.0f*std::max(normalized_f_x, normalized_f_y);

	auto const w_scaled = normalized_f_x > normalized_f_y?
		min_pixel_count: min_pixel_count*size.width/size.height;
	auto const h_scaled = normalized_f_x > normalized_f_y?
		min_pixel_count*size.height/size.width : min_pixel_count;

	auto filter = make_filter(
		2u*std::max(static_cast<uint32_t>(w_scaled + 0.5f), 1u),
		2u*std::max(static_cast<uint32_t>(h_scaled + 0.5f), 1u),
		2.0f*min_pixel_count/params.wavelength_x,
		2.0f*min_pixel_count/params.wavelength_y
	);

	store(filter, "/dev/shm/slask.exr");

	return filter;
}
