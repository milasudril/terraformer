//@	{"dependencies_extra": [{"ref": "./plain.o", "rel": "implementation"}]}"}}

#ifndef TERRAFORMER_PLAIN_GENERATOR_HPP
#define TERRAFORMER_PLAIN_GENERATOR_HPP

#include "lib/generators/domain/domain_size.hpp"

#include "lib/pixel_store/image.hpp"
#include "lib/common/interval.hpp"
#include "lib/common/bounded_value.hpp"

namespace terraformer
{
	struct plain_control_point_descriptor
	{
		float elevation = 840.0f;
		float ddx = 0.0f;
		float ddy = 0.0f;
	};

	struct plain_boundary_descriptor
	{
		plain_control_point_descriptor n;
		plain_control_point_descriptor ne;
		plain_control_point_descriptor e;
		plain_control_point_descriptor se;
		plain_control_point_descriptor s;
		plain_control_point_descriptor sw;
		plain_control_point_descriptor w;
		plain_control_point_descriptor nw;
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
		plain_boundary_descriptor boundary;
		plain_edge_midpoint_descriptor edge_midpoints;
		float orientation = 0.0f;
	};

	struct plain
	{
		grayscale_image z_interp;
		grayscale_image z_grad;
	};

	plain generate(domain_size_descriptor const& dom_size, plain_descriptor const& params);
}

#endif