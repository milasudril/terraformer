//@	{"target":{"name":"./heightmap.o"}}

#include "./heightmap.hpp"

#include "lib/math_utils/interp.hpp"

#include "lib/pixel_store/image_io.hpp"


terraformer::grayscale_image terraformer::generate(heightmap_descriptor const& descriptor)
{
	auto const plain = descriptor.generators.plain.generate_heightmap(descriptor.domain_size);
	auto const rolling_hills = descriptor.generators.rolling_hills.generate_heightmap(descriptor.domain_size);

	auto const output_width = std::max(plain.width(), rolling_hills.width());
	auto const output_height = std::max(plain.height(), 	rolling_hills.height());;

	grayscale_image ret{output_width, output_height};

	add_resampled(plain.pixels(), ret.pixels(), 1.0f);
	add_resampled(rolling_hills.pixels(), ret.pixels(), 1.0f);

	return ret;
}