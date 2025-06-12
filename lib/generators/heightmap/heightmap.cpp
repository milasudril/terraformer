//@	{"target":{"name":"./heightmap.o"}}

#include "./heightmap.hpp"

#include "lib/math_utils/interp.hpp"

#include "lib/pixel_store/image_io.hpp"


terraformer::grayscale_image terraformer::generate(heightmap_descriptor const& descriptor)
{
	auto const plain = generate(descriptor.domain_size, descriptor.generators.plain);
	auto const rolling_hills = generate(descriptor.domain_size, descriptor.generators.rolling_hills);

	auto const output_width = std::max(
		std::max(plain.z_interp.width(), plain.z_grad.width()),
		rolling_hills.width()
	);
	auto const output_height = std::max(
		std::max(plain.z_interp.height(), plain.z_grad.height()),
		rolling_hills.height()
	);

	grayscale_image ret{output_width, output_height};

	{
		add_resampled(plain.z_interp.pixels(), ret.pixels(), 1.0f);
		add_resampled(plain.z_grad.pixels(), ret.pixels(), 1.0f);

		store(plain.z_grad.pixels(), "z_grad.exr");
	}

	add_resampled(rolling_hills.pixels(), ret.pixels(), 1.0f);

	return ret;
}