//@	{"dependencies_extra": [{"ref": "./rolling_hills_generator.o", "rel": "implementation"}]}"}}

#ifndef TERRAFORMER_ROLLING_HILLS_GENERATOR_HPP
#define TERRAFORMER_ROLLING_HILLS_GENERATOR_HPP

#include "lib/generators/domain/domain_size.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/common/interval.hpp"

namespace terraformer
{
	struct rolling_hills_shape_descriptor
	{
		closed_closed_interval<float> clamp_to{-1.0f, 1.0f};
		closed_closed_interval<float> input_mapping{-1.0f, 1.0f};
		float exponent = 1.0f;
	};

	struct rolling_hills_descriptor
	{
		std::array<std::byte, 16> rng_seed{};
		float wavelength_x = 4096.0f;
		float wavelength_y = 4096.0f;
		float filter_orientation = 0.0f;
		rolling_hills_shape_descriptor shape;
		float amplitude = 2048.0f/std::numbers::pi_v<float>;
		float relative_z_offset = 0.0f;
	};

	grayscale_image generate(domain_size_descriptor const& dom_size, rolling_hills_descriptor const& params);
}

#endif