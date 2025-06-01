//@	{"dependencies_extra": [{"ref": "./plain.o", "rel": "implementation"}]}"}}

#ifndef TERRAFORMER_PLAIN_GENERATOR_HPP
#define TERRAFORMER_PLAIN_GENERATOR_HPP

#include "lib/generators/domain/domain_size.hpp"

#include "lib/pixel_store/image.hpp"
#include "lib/common/interval.hpp"
#include "lib/common/bounded_value.hpp"

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

	struct plain_edge_midpoint_descriptor
	{
		using xm_type = bounded_value<open_open_interval{0.0f, 1.0f}, 0.5f>;
		xm_type n;
		xm_type e;
		xm_type s;
		xm_type w;
	};

	struct plain_descriptor
	{
		plain_control_point_elevation_descriptor elevations;
		plain_edge_midpoint_descriptor edge_midpoints;
	};

	grayscale_image generate(domain_size_descriptor const& dom_size, plain_descriptor const& params);
}

#endif