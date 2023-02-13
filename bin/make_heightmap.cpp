//@	{"target":{"name":"make_heightmap.o"}}

#include "lib/curve_tool/ridge_curve.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/filters/curve_rasterizer.hpp"
#include "lib/pixel_store/image_io.hpp"
#include "lib/filters/diffuser.hpp"
#include "lib/execution/thread_pool.hpp"
#include "lib/filters/coordinate_sampler.hpp"
#include "lib/filters/gradient_tracer.hpp"
#include "lib/filters/raycaster.hpp"

#include <random>
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

	struct domain_boundary_conditions
	{
		float front_level;
		float back_level;
	};

	struct massif_outline_descriptor
	{
		domain_boundary_conditions boundary;
		main_ridge_params main_ridge;
	};

	template<class Rng>
	void make_initial_heightmap(double_buffer<terraformer::grayscale_image>& buffers,
		Rng&& rng,
		float pixel_size,
		massif_outline_descriptor const& heightmap_params)
	{
		auto const curve = generate(rng, pixel_size, heightmap_params.main_ridge);
		auto const w = buffers.front().width();
		auto const h = buffers.front().height();

		terraformer::grayscale_image main_ridge{w, h};
		draw(main_ridge.pixels(), curve, terraformer::line_segment_draw_params{
			.value = 1.0f,
			.scale = pixel_size
		});

		generate(buffers.back().pixels(), [
				y_0 = heightmap_params.main_ridge.start_location[1]/pixel_size,
				z_0 = heightmap_params.main_ridge.start_location[2],
				h = static_cast<float>(h),
				boundary = heightmap_params.boundary
			](uint32_t, uint32_t y) {
			auto const y_val = static_cast<float>(y);

			auto const t1 = y_val/y_0;
			auto const t2 = (y_val - y_0)/(h - y_0);

			auto const z1 = std::lerp(boundary.back_level, z_0, t1);
			auto const z2 = std::lerp(z_0, boundary.front_level, t2);

			return t1 < 1.0f ? z1 : z2;
		});
		buffers.swap();

		solve_bvp(buffers, terraformer::laplace_solver_params{
			.tolerance = 1.0e-6f * heightmap_params.main_ridge.start_location[2],
			.step_executor_factory = terraformer::thread_pool_factory{16},
			.boundary = [
				values = main_ridge,
				front_back = heightmap_params.boundary
			](uint32_t x, uint32_t y) {
				if(y == 0)
				{
					return terraformer::dirichlet_boundary_pixel{
						.weight=1.0f,
						.value=front_back.back_level
					};
				}

				if(y == values.height() - 1)
				{
					return terraformer::dirichlet_boundary_pixel{
						.weight=1.0f,
						.value=front_back.front_level
					};
				}

				auto const val = values(x, y);
				return val >= 0.5f ?
					terraformer::dirichlet_boundary_pixel{1.0f, val}:
					terraformer::dirichlet_boundary_pixel{0.0f, 0.0f};
			},
		});
	}

	struct landscape_descriptor
	{
		dimensions physical_dimensions;
		uint32_t pixel_count;
		massif_outline_descriptor initial_heightmap;

#if 0
		noisy_drift::params wind_direction;
		float max_precipitation_rate;

		geosimd::turn_angle north_offset;
		geosimd::rotation_angle center_latitude;
#endif
	};
}

std::pair<uint32_t, uint32_t> pixel_dimensions(float width, float height, uint32_t pixel_count)
{
	auto const w = static_cast<double>(width);
	auto const h = static_cast<double>(height);
	auto const d = static_cast<double>(pixel_count);
	auto const r = w/h;

	return std::pair{
		static_cast<uint32_t>(d*std::sqrt(r) + 0.5),
		static_cast<uint32_t>(d/std::sqrt(r) + 0.5)
	};
}

int main()
{
	terraformer::landscape_descriptor const params{
		.physical_dimensions{
			.width = 49152.0f,
			.height = 49152.0f
		},
		.pixel_count = 1024,
		.initial_heightmap{
			.boundary{
				.front_level = 512.0f,
				.back_level = 2048.0f
			},
			.main_ridge{
				.start_location = terraformer::location{0.0f, 16384.0f, 6144.0f},
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
		}
	};

	auto const pixel_size = static_cast<float>(std::sqrt(domain_area(params.physical_dimensions))
		/static_cast<double>(params.pixel_count));
	auto const canvas_size = pixel_dimensions(
		params.physical_dimensions.width,
		params.physical_dimensions.height,
		params.pixel_count
	);

	fprintf(stderr, "pixel_size: %.8g\n", pixel_size);
	fprintf(stderr, "width: %u\n", canvas_size.first);
	fprintf(stderr, "height: %u\n", canvas_size.second);


	terraformer::double_buffer<terraformer::grayscale_image> buffers{
		canvas_size.first,
		canvas_size.second
	};

	random_generator rng;
	make_initial_heightmap(buffers, rng, pixel_size, params.initial_heightmap);
	store(buffers.front(), "after_laplace.exr");

#if 0
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


	terraformer::grayscale_image lit_surface{canvas_size.first, canvas_size.second};
	generate(lit_surface.pixels(), [heightmap = buffers.front()](uint32_t x, uint32_t y){
		return raycast(heightmap,
			terraformer::pixel_coordinates{x, y},
			heightmap(x, y),
			geosimd::rotation_angle{0x0}).has_value() ? 0.0f : 1.0f;
	});
	store(lit_surface, "lit_surface.exr");

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
