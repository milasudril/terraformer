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

	struct lightmap_params
	{
		hires_location planet_loc;
		geosimd::rotation<hires_geom_space> planet_rot;
		double planet_radius;
		float pixel_size;
		geosimd::rotation_angle center_latitude;
		geosimd::rotation<geom_space> domain_rot;
	};

	inline float intensity(span_2d<float const> heightmap,
					pixel_coordinates loc,
					lightmap_params const& params)
	{
		auto const n = normal(heightmap, loc.x, loc.y, 1.0f/params.pixel_size);
		auto const maploc = to_map_location(
			loc,
			heightmap.extents(),
			params.pixel_size/params.planet_radius
		);

		auto const sun_dir = local_sun_direction(
			params.planet_loc,
			params.planet_rot,
			to_longcolat(maploc, params.center_latitude)
		);

		terraformer::displacement const sun_dir_float{
			static_cast<float>(sun_dir[0]),
			static_cast<float>(sun_dir[1]),
			static_cast<float>(sun_dir[2])
		};

		if(sun_dir[2] <= 0.0)
		{ return 0.0f; }

		auto const d = terraformer::direction(sun_dir_float).apply(params.domain_rot);

		auto const n_proj = inner_product(n, d);

		if(n_proj <= 0.0f)
		{ return 0.0f; }

		auto const raycast_result = raycast(
			heightmap,
			loc,
			heightmap(loc.x, loc.y),
			d,
			// TODO: Use a proper upper limit instead of hardcoding 8192.0f
			[heightmap](auto, auto loc){ return loc[2] <= 8192.0f && inside(heightmap, loc[0], loc[1]);}
		);

		if(raycast_result.has_value())
		{ return 0.0f; }

		return n_proj;
	}

	inline void generate_lightmap(span_2d<float> output_buffer,
		span_2d<float const> heightmap,
		lightmap_params const& params,
		scanline_range range)
	{
		for(uint32_t k = range.begin; k != range.end; ++k)
		{
			for(uint32_t l = 0; l != heightmap.width(); ++l)
			{
				output_buffer(l, k) = intensity(heightmap, pixel_coordinates{.x = l, .y = k}, params);
			}
		}
	}

	lightmap_params make_lightmap_params(year t,
		planet_descriptor const& planetary_data,
		float pixel_size,
		geosimd::rotation_angle center_latitude,
		geosimd::rotation_angle domain_orientation)
	{
		return lightmap_params{
			.planet_loc = planet_location(t, planetary_data.distance_to_sun),
			.planet_rot = planet_rotation(t,
				planetary_data.spin_frequency,
				[&tilt = planetary_data.tilt](year t)
				{
					return tilt.mean + tilt.amplitude*wave_sum{tilt.motion_params}(t.value());
				}),
			.planet_radius = planetary_data.radius,
			.pixel_size = pixel_size,
			.center_latitude = center_latitude,
			.domain_rot = geosimd::rotation<geom_space>{domain_orientation, geosimd::dimension_tag<2>{}}
		};
	}

	inline void generate_lightmap(
		span_2d<float> output,
		span_2d<float const> heightmap,
		year t,
		planet_descriptor const& planetary_data,
		float pixel_size,
		geosimd::rotation_angle center_latitude,
		geosimd::rotation_angle domain_orientation)
	{
		generate_lightmap(output,
			heightmap,
			make_lightmap_params(t, planetary_data, pixel_size, center_latitude, domain_orientation),
			scanline_range{0, heightmap.height()});
	}
}

#endif