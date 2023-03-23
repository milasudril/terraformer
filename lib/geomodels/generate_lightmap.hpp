#ifndef TERRAFORMER_LIB_GENERATE_LIGHTMAP_HPP
#define TERRAFORMER_LIB_GENERATE_LIGHTMAP_HPP

#include "./sun_direction.hpp"
#include "lib/curve_tool/wave_sum.hpp"
#include "lib/filters/raycaster.hpp"

namespace terraformer
{
	struct planet_tilt_params
	{
		geosimd::turn_angle mean;
		geosimd::turn_angle amplitude;
		wave_sum<double>::params motion_params;
	};

	struct planet_descriptor
	{
		double distance_to_sun;
		double radius;
		double spin_frequency;
		planet_tilt_params tilt;
	};

	inline year to_years(double days, planet_descriptor const& planetary_data)
	{
		return year{days/planetary_data.spin_frequency};
	}

	inline void generate_lightmap(
		span_2d<float> output,
		span_2d<float const> heightmap,
		year t,
		planet_descriptor const& planetary_data,
		float pixel_size,
		geosimd::rotation_angle center_latitude,
		geosimd::rotation_angle domain_orientation
	)
	{
		generate(output, [
			heightmap,
			planet_loc = planet_location(t, planetary_data.distance_to_sun),
			planet_rot = planet_rotation(t,
				planetary_data.spin_frequency,
				[&tilt = planetary_data.tilt](year t)
				{
					return tilt.mean + tilt.amplitude*wave_sum{tilt.motion_params}(t.value());
				}),
			planet_radius = planetary_data.radius,
			pixel_size,
			center_latitude,
			dom_rot = geosimd::rotation<geom_space>{domain_orientation, geosimd::dimension_tag<2>{}}
		]
		(uint32_t x, uint32_t y){
			auto const n = normal(heightmap, x, y, 1.0f/pixel_size);
			auto const maploc = to_map_location(
				terraformer::pixel_coordinates{x, y},
				heightmap.extents(),
				pixel_size/planet_radius
			);

			auto const sun_dir = local_sun_direction(
				planet_loc,
				planet_rot,
				to_longcolat(maploc, center_latitude)
			);

			terraformer::displacement const sun_dir_float{
				static_cast<float>(sun_dir[0]),
				static_cast<float>(sun_dir[1]),
				static_cast<float>(sun_dir[2])
			};

			if(sun_dir[2] <= 0.0)
			{ return 0.0f; }

			auto const d = terraformer::direction(sun_dir_float).apply(dom_rot);

			auto const n_proj = inner_product(n, d);

			if(n_proj <= 0.0f)
			{ return 0.0f; }

			auto const raycast_result = raycast(
				heightmap,
				terraformer::pixel_coordinates{x, y},
				heightmap(x, y),
				d,
				[heightmap](auto, auto loc){ return loc[2] <= 8192.0f && inside(heightmap, loc[0], loc[1]);}
			);

			if(raycast_result.has_value())
			{ return 0.0f; }

			return n_proj;
		});
	}
}

#endif