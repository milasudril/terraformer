//@	{"dependencies_extra":[{"ref":"./heigtmap.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_HEIGHTMAP_HPP
#define TERRAFORMER_HEIGHTMAP_HPP

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

	grayscale_image generate(heightmap_descriptor const& descriptor);
}

#endif