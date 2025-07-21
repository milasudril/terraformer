//@	{"dependencies_extra": [{"ref": "./plain.o", "rel": "implementation"}]}"}}

#ifndef TERRAFORMER_PLAIN_GENERATOR_HPP
#define TERRAFORMER_PLAIN_GENERATOR_HPP

#include "lib/generators/domain/domain_size.hpp"

#include "lib/pixel_store/image.hpp"
#include "lib/common/interval.hpp"
#include "lib/common/bounded_value.hpp"
#include "lib/descriptor_io/descriptor_editor_ref.hpp"

namespace terraformer
{
	struct plain_control_point_descriptor
	{
		float elevation = 840.0f;
		float ddx = 0.0f;
		float ddy = 0.0f;

		bool operator==(plain_control_point_descriptor const&) const = default;
		bool operator!=(plain_control_point_descriptor const&) const = default;

		void bind(descriptor_editor_ref editor);
	};

	struct plain_control_points_info
	{
		plain_control_point_descriptor n;
		plain_control_point_descriptor ne;
		plain_control_point_descriptor e;
		plain_control_point_descriptor se;
		plain_control_point_descriptor s;
		plain_control_point_descriptor sw;
		plain_control_point_descriptor w;
		plain_control_point_descriptor nw;
		plain_control_point_descriptor c;

		bool operator==(plain_control_points_info const&) const = default;
		bool operator!=(plain_control_points_info const&) const = default;

		void bind(descriptor_table_editor_ref editor);
	};

	struct plain_midpoints_info
	{
		using xm_type = bounded_value<open_open_interval{0.0f, 1.0f}, 0.5f>;
		xm_type n;
		xm_type e;
		xm_type s;
		xm_type w;
		xm_type c_x;
		xm_type c_y;

		bool operator==(plain_midpoints_info const&) const = default;
		bool operator!=(plain_midpoints_info const&) const = default;

		void bind(descriptor_editor_ref editor);
	};

	struct plain_descriptor
	{
		plain_control_points_info control_points;
		plain_midpoints_info midpoints;

		grayscale_image generate_heightmap(domain_size_descriptor) const;
		void bind(descriptor_editor_ref editor);

		bool operator==(plain_descriptor const&) const = default;
		bool operator!=(plain_descriptor const&) const = default;
	};

	grayscale_image generate(domain_size_descriptor dom_size, plain_descriptor const& params);
}

#endif
