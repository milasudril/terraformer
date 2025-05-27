//@	{"target": {"name": "./rolling_hills_generator.o"}}

#include "./rolling_hills_generator.hpp"

terraformer::grayscale_image
terraformer::generate(domain_size_descriptor const& size, rolling_hills_descriptor const& params)
{
	auto const size_factor = std::min(size.width, size.height);
	auto const normalized_lambda_x = size.width/params.wavelength_x;
	auto const normalized_lambda_y = size.height/params.wavelength_y;
	// Assume a bandwidth of at most 6 octaves = 64 periods
	// Take 4 samples per period
	// This gives a size of 256 pixels, but the size is multiplied by 2 to guarantee an even number.
	// Therefore, use 128 pixels as factor
	auto const min_pixel_count = 128.0f*std::max(normalized_lambda_x, normalized_lambda_y);
	auto const w_scaled = min_pixel_count*size.width/size_factor;
	auto const h_scaled = min_pixel_count*size.height/size_factor;

	return grayscale_image{
		2*static_cast<uint32_t>(w_scaled + 0.5f),
		2*static_cast<uint32_t>(h_scaled + 0.5f)
	};
}
