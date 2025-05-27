//@	{"dependencies_extra": [{"ref": "./rolling_hills_generator.o", "rel": "implementation"}]}"}}

#ifndef TERRAFORMER_ROLLING_HILLS_GENERATOR_HPP
#define TERRAFORMER_ROLLING_HILLS_GENERATOR_HPP

#include "lib/generators/domain/domain_size.hpp"
#include "lib/pixel_store/image.hpp"

namespace terraformer
{
	struct rolling_hills_descriptor
	{
		float wavelength_x = 4096.0f;
		float wavelength_y = 4096.0f;
	};

	grayscale_image generate(domain_size_descriptor const& dom_size, rolling_hills_descriptor const& params);
}

#endif