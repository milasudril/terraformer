//@	{"dependencies_extra": [{"ref": "./plain.o", "rel": "implementation"}]}"}}

#ifndef TERRAFORMER_PLAIN_GENERATOR_HPP
#define TERRAFORMER_PLAIN_GENERATOR_HPP

#include "lib/common/domain_size.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/pixel_store/image.hpp"

namespace terraformer
{
	struct plain_corner_descriptor
	{
		float elevation = 840.0f;
		float ddx = 0.0f;
		float ddy = 0.0f;
	};

	struct plain_descriptor
	{
		plain_corner_descriptor nw;
		plain_corner_descriptor ne;
		plain_corner_descriptor sw;
		plain_corner_descriptor se;
	};

	void replace_pixels(
		span_2d<float> output,
		float pixel_size,
		plain_descriptor const& params
	);

	grayscale_image generate(domain_size const& dom_size, plain_descriptor const& params);
}

#endif