#ifndef TERRAFORMER_GENERATE_LIGHTMAP_HPP
#define TERRAFORMER_GENERATE_LIGHTMAP_HPP

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
		hires_rotation planet_rot;
		double planet_radius;
		float pixel_size;
		geosimd::rotation_angle center_latitude;
		rotation domain_rot;
	};

	inline lightmap_params make_lightmap_params(year t,
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
			.domain_rot = rotation{domain_orientation, geosimd::dimension_tag<2>{}}
		};
	}

	inline float intensity(span_2d<float const> heightmap,
					span_2d<float const> upper_boundary,
					pixel_coordinates loc,
					lightmap_params const& params)
	{
		auto const n = normal(heightmap, loc.x, loc.y, 1.0f/params.pixel_size, clamp_at_boundary{});
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

		// NOTE: It is assumed that domain is only rotated around z axis. Thus, it is ok to discard
		//       the current fragment if sun is below horizon before applying domain rotation.
		if(sun_dir[2] <= 0.0)
		{ return 0.0f; }

		terraformer::displacement const sun_dir_float{
			static_cast<float>(sun_dir[0]),
			static_cast<float>(sun_dir[1]),
			static_cast<float>(sun_dir[2])
		};

		auto const d = terraformer::direction(sun_dir_float).apply(params.domain_rot);

		auto const n_proj = inner_product(n, d);

		if(n_proj <= 0.0f)
		{ return 0.0f; }

		auto const raycast_result = raycast(
			heightmap,
			loc,
			heightmap(loc.x, loc.y),
			d,
			params.pixel_size,
			[heightmap, upper_boundary](auto, auto loc){
				if(!inside(heightmap, loc[0], loc[1]))
				{ return raycast_pred_result::stop; }

				if(loc[2] > interp(upper_boundary, loc[0], loc[1], clamp_at_boundary{}))
				{ return raycast_pred_result::stop; }

				return raycast_pred_result::keep_going;
			}
		);

		if(raycast_result.has_value())
		{ return 0.0f; }

		return n_proj;
	}

	inline void generate_lightmap(span_2d<float> output_buffer,
		span_2d<float const> heightmap,
		span_2d<float const> upper_boundary,
		lightmap_params const& params,
		scanline_range range)
	{
		for(uint32_t k = range.begin; k != range.end; ++k)
		{
			for(uint32_t l = 0; l != heightmap.width(); ++l)
			{
				output_buffer(l, k) = intensity(heightmap, upper_boundary, pixel_coordinates{.x = l, .y = k}, params);
			}
		}
	}

	inline void generate_lightmap(
		span_2d<float> output,
		span_2d<float const> heightmap,
		span_2d<float const> upper_boundary,
		year t,
		planet_descriptor const& planetary_data,
		float pixel_size,
		geosimd::rotation_angle center_latitude,
		geosimd::rotation_angle domain_orientation)
	{
		generate_lightmap(output,
			heightmap,
			upper_boundary,
			make_lightmap_params(t, planetary_data, pixel_size, center_latitude, domain_orientation),
			scanline_range{0, heightmap.height()});
	}

	using lightmap_generation_pass = notifying_task<
		std::reference_wrapper<signaling_counter>,
		void (*)(span_2d<float>, span_2d<float const>, span_2d<float const>, lightmap_params const&, scanline_range),
		span_2d<float>,
		span_2d<float const>,
		span_2d<float const>,
		std::reference_wrapper<lightmap_params const> const,
		scanline_range>;

	template<class ExecutorFactory>
	class lightmap_generator
	{
	public:
		using step_exec_type = lightmap_generation_pass;
		using executor_type = decltype(std::declval<ExecutorFactory>()(empty<step_exec_type>{}));

		explicit lightmap_generator(ExecutorFactory&& exec_factory,
			span_2d<float> output_buffer,
			span_2d<float const> input_buffer,
			span_2d<float const> upper_boundary,
			lightmap_params const& params):
			m_executor{exec_factory(empty<step_exec_type>{})},
			m_output_buffer{output_buffer},
			m_input_buffer{input_buffer},
			m_upper_boundary{upper_boundary},
			m_params{params}
		{}

		auto operator()()
		{
			auto const n_workers = std::size(m_executor);

			auto const domain_height = m_output_buffer.height();
			auto const batch_size = 1 + (domain_height - 1)/static_cast<uint32_t>(n_workers);
			signaling_counter counter{n_workers};
			for(size_t k = 0; k != n_workers; ++k)
			{
				m_executor.run(notifying_task{
					std::ref(counter),
					+[](span_2d<float> output_buffer,
						span_2d<float const> heightmap,
						span_2d<float const> upper_boundary,
						lightmap_params const& params,
						scanline_range range)
					{
						return generate_lightmap(output_buffer, heightmap, upper_boundary, params, range);
					},
					m_output_buffer,
					m_input_buffer,
					m_upper_boundary,
					std::cref(m_params),
					scanline_range{
						.begin = static_cast<uint32_t>(k*batch_size),
						.end = std::min(domain_height, static_cast<uint32_t>((k + 1)*batch_size))
					}
				});
			}
			counter.wait();
		}

	private:
		executor_type m_executor;
		span_2d<float> m_output_buffer;
		span_2d<float const> m_input_buffer;
		span_2d<float const> m_upper_boundary;
		lightmap_params m_params;
	};
}

#endif