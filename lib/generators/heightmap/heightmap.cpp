//@	{"target":{"name":"./heightmap.o"}}

#include "./heightmap.hpp"

#include "lib/math_utils/interp.hpp"

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
		auto const w_in = static_cast<float>(plain.z_grad.width());
		auto const h_in = static_cast<float>(plain.z_grad.height());
		add_resampled(plain.z_interp.pixels(), ret.pixels(), 1.0f);
		auto const scale = std::sqrt(
			(static_cast<float>(output_width)*static_cast<float>(output_height))/
			(w_in*h_in)
		);
		add_resampled(plain.z_grad.pixels(), ret.pixels(), 1.0f*scale);
	}

	add_resampled(rolling_hills.pixels(), ret.pixels(), 1.0f);

	return ret;
}