#ifndef TERRAFORMER_HEIGHTMAP_DESCRIPTOR_HPP
#define TERRAFORMER_HEIGHTMAP_DESCRIPTOR_HPP

#include "lib/generators/plain_generator/plain.hpp"
#include "lib/generators/rolling_hills_generator/rolling_hills_generator.hpp"
#include "lib/generators/domain/domain_size.hpp"

namespace terraformer
{
	struct heightmap_generator_descriptor
	{
		plain_descriptor plain;
		rolling_hills_descriptor rolling_hills;
	};

	struct heightmap_descriptor
	{
		domain_size_descriptor domain_size;
		heightmap_generator_descriptor generators;
	};

	inline grayscale_image generate(heightmap_descriptor const& descriptor)
	{
		auto const plain = generate(descriptor.domain_size, descriptor.generators.plain);
		auto const rolling_hills = generate(descriptor.domain_size, descriptor.generators.rolling_hills);

		auto const w_max = std::max(plain.width(), rolling_hills.width());
		auto const h_max = std::max(plain.height(), rolling_hills.height());


		printf("%u %u\n", w_max, h_max);

		return plain;
	}
}

#endif