//@	{"dependencies_extra": [{"ref": "./plain.o", "rel": "implementation"}]}"}}

#ifndef TERRAFORMER_PLAIN_GENERATOR_HPP
#define TERRAFORMER_PLAIN_GENERATOR_HPP

#include "lib/generators/domain/domain_size.hpp"
#include "lib/pixel_store/image.hpp"

namespace terraformer
{
	struct plain_control_point_elevation_descriptor
	{
		float n = 840.0f;
		float ne = 840.0f;
		float e = 840.0f;
		float se = 840.0f;
		float s = 840.0f;
		float sw = 840.f;
		float w = 840.0f;
		float nw = 840.0f;
	};

	struct plain_descriptor_new
	{
		plain_control_point_elevation_descriptor elevations;
	};

	grayscale_image generate(domain_size_descriptor const& dom_size, plain_descriptor_new const& params);

	struct plain_edge_descriptor
	{
		float xi_0;
		float elevation;
	};

	struct plain_corner_descriptor
	{
		float elevation = 840.0f;
		float ddx = 0.0f;
		float ddy = 0.0f;
	};

	struct plain_descriptor
	{
		float n = 840.0f;
		plain_corner_descriptor ne;
		float e = 840.0f;
		plain_corner_descriptor se;
		float s = 840.0f;
		plain_corner_descriptor sw;
		float w = 840.0f;
		plain_corner_descriptor nw;
		float midpoint = 840.0f;
	};

	void replace_pixels(
		span_2d<float> output,
		float pixel_size,
		plain_descriptor const& params
	);

	grayscale_image generate(domain_size_descriptor const& dom_size, plain_descriptor const& params);
}

#endif