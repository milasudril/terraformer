#ifndef TERRAFORMER_HEIGHTMAP_DESCRIPTOR_HPP
#define TERRAFORMER_HEIGHTMAP_DESCRIPTOR_HPP

#include "lib/generators/plain_generator/plain.hpp"
#include "lib/generators/domain/domain_size.hpp"

namespace terraformer
{
	struct heightmap_generator_descriptor
	{
		plain_descriptor plain;
	};

	struct heightmap_descriptor
	{
		domain_size_descriptor domain_size;
		heightmap_generator_descriptor generators;
	};
}

#endif