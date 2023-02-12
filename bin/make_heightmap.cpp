//@	{"target":{"name":"make_heightmap.o"}}

#include "lib/curve_tool/ridge_curve.hpp"

#include "lib/curve_tool/noisy_drift.hpp"
#include "lib/curve_tool/damped_motion_integrator.hpp"
#include "lib/curve_tool/turn_angle_limiter.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/filters/curve_rasterizer.hpp"
#include "lib/pixel_store/image_io.hpp"
#include "lib/filters/diffuser.hpp"
#include "lib/execution/thread_pool.hpp"
#include "lib/filters/coordinate_sampler.hpp"
#include "lib/filters/gradient_tracer.hpp"

#include <random>
#include <pcg-cpp/include/pcg_random.hpp>

using random_generator = pcg_engines::oneseq_dxsm_128_64;

namespace terraformer
{
	struct dimensions
	{
		float width;
		float height;
		float thickness;
	};

	struct domain_boundary_conditions
	{
		float low_level;
		float high_level;
	};

	struct massif_outline_description
	{
		domain_boundary_conditions boundary;
		main_ridge_params main_ridge;
	};

	struct landscape_description
	{
		dimensions physical_dimensions;
		uint32_t pixel_count;
		massif_outline_description initial_heightmap_description;

#if 0
		noisy_drift::params wind_direction;
		float max_precipitation_rate;

		geosimd::turn_angle north_offset;
		geosimd::rotation_angle center_latitude;
#endif
	};
}

int main()
{
	uint32_t const domain_size = 1024;

	terraformer::location const r_0{0.0f, 1.0f*static_cast<float>(domain_size)/3.0f, 1.0f};

	random_generator rng;

	auto const curve = generate(rng, terraformer::main_ridge_params{
		.start_location = r_0,
		.distance_to_endpoint = static_cast<float>(domain_size),
		.wave_params = terraformer::fractal_wave::params{
			.wavelength = 512.0f,
			.per_wave_component_scaling_factor = std::numbers::phi_v<float>,
			.exponent_noise_amount = std::numbers::phi_v<float>/16.0f,
			.per_wave_component_phase_shift = 2.0f - std::numbers::phi_v<float>,
			.phase_shift_noise_amount = 1.0f/12.0f
		},
		.wave_amplitude = 4096.0f*1024.0f/49152.0f,
		.height_modulation = 1024.0f/6144.0f
	});


	// Generate initial heightmap

	terraformer::grayscale_image boundary_values{domain_size, domain_size};
	draw(boundary_values.pixels(), curve, terraformer::line_segment_draw_params{.value = 1.0f});
	store(boundary_values, "boundary.exr");

	terraformer::double_buffer<terraformer::grayscale_image> buffers{domain_size, domain_size};
	generate(buffers.back().pixels(), [r_0](uint32_t, uint32_t y) {
		auto const y_val = static_cast<float>(y);
		auto const ridge_line = r_0[1];
		auto const t = y_val/ridge_line;
		return std::min(std::lerp(0.382f, 1.0f, t),
			std::lerp(1.0f, 0.618f*0.382f, (y_val - ridge_line)/static_cast<float>(domain_size - ridge_line)));
	});

	buffers.swap();
	store(buffers.front(), "initial_state.exr");

	solve_bvp(buffers, terraformer::laplace_solver_params{
		.tolerance = 1.0e-6f,
		.step_executor_factory = terraformer::thread_pool_factory{16},
		.boundary = [values = boundary_values](uint32_t x, uint32_t y) {
			if(y == 0)
			{ return terraformer::dirichlet_boundary_pixel{.weight=1.0f, .value=0.382f}; }

			if(y == values.height() - 1)
			{ return terraformer::dirichlet_boundary_pixel{.weight=1.0f, .value=0.618f*0.382f};}

			auto const val = values(x, y);
			return val >= 0.5f ?
				terraformer::dirichlet_boundary_pixel{1.0f, val}:
				terraformer::dirichlet_boundary_pixel{0.0f, 0.0f};
		},
	});

	store(buffers.front(), "after_laplace.exr");

	auto const heightmap = buffers.front().pixels();
	// Collect river start points
	auto river_start_points = terraformer::sample(domain_size,
		domain_size,
		[&rng, heightmap](uint32_t x, uint32_t y){
			std::uniform_real_distribution U{0.0f, 1.0f};
			// TODO: normalize to max value in heightmap
			auto const val = 0.75f*heightmap(x, y);

			// TODO: These numbers constants should be parameters
			return 768.0f*U(rng) < (val >= 0.75f);
		});

	std::ranges::shuffle(river_start_points, rng);

	terraformer::grayscale_image river_mask{domain_size, domain_size};
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
}
