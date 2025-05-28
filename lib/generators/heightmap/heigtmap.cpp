//@	{"target":{"name":"./heigtmap.o"}}

#include "./heightmap.hpp"

terraformer::grayscale_image terraformer::generate(heightmap_descriptor const& descriptor)
{
	auto const plain = generate(descriptor.domain_size, descriptor.generators.plain);
	auto const rolling_hills = generate(descriptor.domain_size, descriptor.generators.rolling_hills);

	auto const w_max = std::max(plain.width(), rolling_hills.width());
	auto const h_max = std::max(plain.height(), rolling_hills.height());

	grayscale_image ret{w_max, h_max};

	return plain;
}