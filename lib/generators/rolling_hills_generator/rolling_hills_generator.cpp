//@	{"target": {"name": "./rolling_hills_generator.o"}}

#include "./rolling_hills_generator.hpp"

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

	return grayscale_image{
		2*static_cast<uint32_t>(w_scaled + 0.5f),
		2*static_cast<uint32_t>(h_scaled + 0.5f)
	};
}
