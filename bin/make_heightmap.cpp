//@	{"target":{"name":"make_heightmap.o"}}

#include "lib/geomodels/steady_plate_collision_zone.hpp"
#include "lib/geomodels/generate_lightmap.hpp"
#include "lib/pixel_store/image_io.hpp"
#include "lib/filters/coordinate_sampler.hpp"
#include "lib/filters/gradient_tracer.hpp"
#include "lib/filters/raycaster.hpp"
#include "lib/filters/convhull.hpp"
#include "lib/curve_tool/wave_sum.hpp"

#include <random>
#include <chrono>
#include <pcg-cpp/include/pcg_random.hpp>

using random_generator = pcg_engines::oneseq_dxsm_128_64;

namespace terraformer
{
	struct dimensions
	{
		float width;
		float height;
	};

	constexpr auto domain_area(dimensions const& dim)
	{
		return static_cast<double>(dim.width)*static_cast<double>(dim.height);
	}

	struct domain_size_descriptor
	{
		dimensions physical_dimensions;
		uint32_t pixel_count;
	};

	inline auto get_canvas_size(domain_size_descriptor const& domain_size)
	{
		auto const w = static_cast<double>(domain_size.physical_dimensions.width);
		auto const h = static_cast<double>(domain_size.physical_dimensions.height);
		auto const d = static_cast<double>(domain_size.pixel_count);
		auto const r = w/h;

		return span_2d_extents{
			static_cast<uint32_t>(d*std::sqrt(r) + 0.5),
			static_cast<uint32_t>(d/std::sqrt(r) + 0.5)
		};
	}

	inline auto get_pixel_size(domain_size_descriptor const& domain_size)
	{
		return static_cast<float>(std::sqrt(domain_area(domain_size.physical_dimensions))
			/static_cast<double>(domain_size.pixel_count));
	}

	inline constexpr auto north = geosimd::rotation_angle{0xc000'0000};
	inline constexpr auto north_east = geosimd::rotation_angle{0xe000'0000};
	inline constexpr auto east = geosimd::rotation_angle{0x0};
	inline constexpr auto south_east = geosimd::rotation_angle{0x2000'0000};
	inline constexpr auto south = geosimd::rotation_angle{0x4000'0000};
	inline constexpr auto south_west = geosimd::rotation_angle{0x6000'0000};
	inline constexpr auto west = geosimd::rotation_angle{0x8000'0000};
	inline constexpr auto north_west = geosimd::rotation_angle{0xa000'0000};

	struct wind_direction_descriptor
	{
		geosimd::rotation_angle expected_value;
		float std_dev;
	};

	struct weather_data_descriptor
	{
		wind_direction_descriptor wind_direction;
	};

	struct domain_descriptor
	{
		domain_size_descriptor size;
		geosimd::rotation_angle center_latitude;
		geosimd::rotation_angle orientation;
	};

	struct landscape_descriptor
	{
		domain_descriptor domain;
		steady_plate_collision_zone_descriptor initial_heightmap;
		planet_descriptor planetary_data;
		weather_data_descriptor weather_data;


#if 0
		noisy_drift::params wind_direction;
		float max_precipitation_rate;

		geosimd::rotation_angle center_latitude;
#endif
	};
}


int main()
{
	terraformer::landscape_descriptor const params{
		.domain{
			.size{
				.physical_dimensions{
					.width = 49152.0f,
					.height = 49152.0f
				},
				.pixel_count = 1024
			},
			.center_latitude = geosimd::rotation_angle{0x2000'0000},
			.orientation = geosimd::rotation_angle{0x0000'0000}
		},
		.initial_heightmap{
			.boundary{
				.front_level = 1024.0f,
				.back_level = 3072.0f
			},
			.main_ridge{
				.start_location = terraformer::location{0.0f, 16384.0f, 8192.0f},
				.distance_to_endpoint = 49152.0f,
				.wave_params{
					.wavelength = 24576.0f,
					.per_wave_component_scaling_factor = std::numbers::phi_v<float>,
					.exponent_noise_amount = std::numbers::phi_v<float>/16.0f,
					.per_wave_component_phase_shift = 2.0f - std::numbers::phi_v<float>,
					.phase_shift_noise_amount = 1.0f/12.0f
				},
				.wave_amplitude = 4096.0f,
				.height_modulation = 1024.0f
			}
		},
		.planetary_data{
			.distance_to_sun = 1.4812e11,
			.radius = 6371000.0,
			.spin_frequency = 366.2563986330786,
			.tilt{
				.mean{geosimd::turn_angle{geosimd::turns{0.06472222222222222}}},
				.amplitude{geosimd::turn_angle{geosimd::turns{0.003333333333333331}}},
				.motion_params{
					.base_frequency = 1.0/41000.0,
					.frequency_ratio = std::sqrt(2)/std::sqrt(3),
					.phase_difference = 0.0,
					.mix = 0.25
				}
			}
		},
		.weather_data{
			.wind_direction{
				.expected_value = terraformer::south_west,
				.std_dev = 1.0f/24.0f
			}
		}
	};

	auto const pixel_size = get_pixel_size(params.domain.size);
	auto const canvas_size = get_canvas_size(params.domain.size);

	fprintf(stderr, "pixel_size: %.8g\n", pixel_size);
	fprintf(stderr, "width: %u\n", canvas_size.width);
	fprintf(stderr, "height: %u\n", canvas_size.height);


	terraformer::double_buffer<terraformer::grayscale_image> buffers{
		canvas_size.width,
		canvas_size.height
	};

	random_generator rng;
	terraformer::default_thread_pool threads{16};

	make_heightmap(buffers, rng, std::ref(threads), pixel_size, params.initial_heightmap);
	putchar('\n');
	store(buffers.front(), "after_laplace.exr");

	terraformer::grayscale_image lightmap{canvas_size.width, canvas_size.height};

	auto const dt = 1.0/(48.0*params.planetary_data.spin_frequency);
	size_t k = 0;
	while(static_cast<double>(k)*dt <= 1.0)
	{
		terraformer::lightmap_generator gen{
			std::ref(threads),
			buffers.back(),
			buffers.front(),
			make_lightmap_params(
				terraformer::year{static_cast<double>(k)*dt},
				params.planetary_data,
				pixel_size,
				params.domain.center_latitude,
				params.domain.orientation)
		};

		gen();

		std::array<char, 32> buffer{};
		sprintf(buffer.data(), "__dump/lightmap_%04zu.exr", k);
		store(buffers.back(), std::as_const(buffer).data());
		printf("%zu                \r", k);
		fflush(stdout);
		++k;
	}
	putchar('\n');

#if 0
	auto hm_conv_hull = buffers.front();

	{
		auto const t0 = std::chrono::steady_clock::now();
		printf("Generating convex hull... ");
		convhull(hm_conv_hull.pixels());
		printf("%.8g s\n", std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
	}
	store(hm_conv_hull, "hm_conv_hull.exr");


	terraformer::grayscale_image lit_surface{canvas_size.width, canvas_size.height};
	{
		auto const t0 = std::chrono::steady_clock::now();
		printf("Generating precipitation data... ");
		generate(lit_surface.pixels(), [heightmap = buffers.front().pixels(),
			&rng,
			wind_dir_distrib = std::normal_distribution{0.0f, params.weather_data.wind_direction.std_dev},
			wind_direction = params.weather_data.wind_direction.expected_value,
			hm_conv_hull = std::as_const(hm_conv_hull).pixels()
		](uint32_t x, uint32_t y) mutable {
			auto const src_dir = terraformer::direction{cossin(
				wind_direction + geosimd::turn_angle{geosimd::turns{wind_dir_distrib(rng)}}
			), geosimd::dimension_tag<2>{}};

			auto const cloud_base = 3072.0f;
			return raycast(
				heightmap,
				terraformer::pixel_coordinates{x, y},
				std::max(heightmap(x, y), cloud_base),
				src_dir,
				[hm_conv_hull](size_t, terraformer::location loc){
					if(!inside(hm_conv_hull, loc[0], loc[1]))
					{ return true; }

					return interp(hm_conv_hull, loc[0], loc[1]) < loc[2];
				}).has_value() ? 0.0f : 1.0f;
		});
		printf("%.8g s\n", std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
	}
	printf("Saving result\n");
	store(lit_surface, "lit_surface.exr");

	store(buffers.front(), "after_laplace.exr");
	auto const heightmap = buffers.front().pixels();
	// Collect river start points
	auto river_start_points = terraformer::sample(canvas_size.first,
		canvas_size.second,
		[&rng, heightmap](uint32_t x, uint32_t y){
			std::uniform_real_distribution U{0.0f, 1.0f};
			// TODO: normalize to max value in heightmap
			auto const val = 0.75f*heightmap(x, y)/6144.0f;

			// TODO: These constants should be parameters
			return 768.0f*U(rng) < (val >= 0.75f);
		});

	std::ranges::shuffle(river_start_points, rng);
#endif



#if 0
	std::ranges::for_each(river_start_points,
		[river_mask = river_mask.pixels(), heightmap](auto const item) {
		auto const path = trace_gradient_periodic_xy(heightmap, item);
		draw(river_mask, get<0>(path), terraformer::line_segment_draw_params{
			.value = 1.0f/32.0f,
			.brush = [](float x, float y) {
				auto const d = x*x + y*y;
				return d<=1.0f ? 1.0f - std::sqrt(d): 0.0f;
			},
			.blend_function = [](float in, float value, float strength){
				return std::max(in, value*strength);
			},
			.brush_diameter = [](float, float){ return 16.0f;}
		});
	});
	transform(heightmap, std::as_const(river_mask).pixels(), buffers.back().pixels(), [](float a, float b) {
		return a - b;
	});
	buffers.swap();
	store(buffers.front(), "eroded.exr");
#endif
}
