//@	{"target":{"name":"curve_gen.o"}}

#include "lib/curve_tool/damped_motion_integrator.hpp"
#include "lib/curve_tool/noisy_drift.hpp"
#include "lib/curve_tool/turn_angle_limiter.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/filters/curve_rasterizer.hpp"
#include "lib/pixel_store/image_io.hpp"
#include "lib/filters/diffuser.hpp"
#include "lib/common/thread_pool.hpp"

#include <random>
#include <pcg-cpp/include/pcg_random.hpp>

using random_generator = pcg_engines::oneseq_dxsm_128_64;

int main()
{
	uint32_t const domain_size = 1024;
	auto const curve_scaling_factor = 6.0f;

	terraformer::location const r_0{0.0f, 2.0f*static_cast<float>(domain_size)/3.0f, 0.0f};

	terraformer::noisy_drift drift{terraformer::noisy_drift::params{
		.drift = geosimd::rotation_angle{0x0},
		.noise_amount = 0.875f
	}};
	terraformer::turn_angle_limiter limiter{
		terraformer::turn_angle_limiter::state{
			.r = r_0,
			.dir = terraformer::direction{terraformer::geom_space::x{}},
			.integrated_heading_change = geosimd::turn_angle{0x0}
		},
		terraformer::turn_angle_limiter::params{
			.max_turn_angle = geosimd::turn_angle{0x1000'0000}
		}
	};
	terraformer::damped_motion_integrator integrator{
		terraformer::displacement{1.0f, 0.0f, 0.0f},
		terraformer::damped_motion_integrator::params{
			.inertia = 2.0f,
			.curve_scaling_factor = curve_scaling_factor
		}
	};

	random_generator rng;

	std::vector<terraformer::location> curve;
	terraformer::particle_state ps{
		.v = terraformer::displacement{0.0f, 0.0f, 0.0f},
		.r = r_0
	};

	curve.push_back(ps.r);
	while(ps.r[0] < static_cast<float>(domain_size))
	{
		auto const v = drift(rng);
		auto const ps_new = integrator(ps, v);
		auto const r_corrected = limiter(ps_new.r);

		auto const v_corr = 2.0f*(r_corrected - ps.r)/curve_scaling_factor - ps.v;

		ps.v = v_corr;
		ps.r = r_corrected;

		curve.push_back(r_corrected + terraformer::displacement{0.0f, 0.0f, 1.0f});
	}

	terraformer::grayscale_image boundary_values{domain_size, domain_size};
	draw_as_line_segments(curve, boundary_values.pixels());
	store(boundary_values, "boundary.exr");

	terraformer::grayscale_image img_a{domain_size, domain_size};
	for(uint32_t y = 0; y != domain_size; ++y)
	{
		for(uint32_t x = 0; x != domain_size; ++x)
		{
			auto const y_val = static_cast<float>(y);
			auto const ridge_line = static_cast<float>(domain_size) - r_0[1];
			auto const t = y_val/ridge_line;
			auto const val = std::min(std::lerp(0.382f, 1.0f, t),
				std::lerp(1.0f, 0.618f*0.382f, (y_val - ridge_line)/static_cast<float>(domain_size - ridge_line)));

			img_a(x, y) = val;
		}
	}

	auto diffuser = make_diffusion_solver<terraformer::thread_pool>(16,
		std::as_const(img_a).pixels(),
		terraformer::diffusion_params{
			.D = 1.0f,
			.boundary = [values = boundary_values](uint32_t x, uint32_t y) {
				if(y == 0)
				{
					return terraformer::dirichlet_boundary_pixel{.weight=1.0f, .value=0.382f};
				}

				if(y == values.height() - 1)
				{
					return terraformer::dirichlet_boundary_pixel{.weight=1.0f, .value=0.618f*0.382f};
				}

				auto const val = values(x, y);
				return val >= 0.5f ?
					terraformer::dirichlet_boundary_pixel{1.0f, val}:
					terraformer::dirichlet_boundary_pixel{0.0f, 0.0f};
			},
			.source =  [](uint32_t, uint32_t){ return 0.0f; }
		}
	);

	while(true)
	{
		auto const delta = diffuser();

		if(delta < 1.0e-6f)
		{ break; }
	}

	store(diffuser.get_buffer(), "test.exr");

#if 0
	terraformer::grayscale_image img_b{domain_size, domain_size};
	auto input_buffer = img_a.pixels();
	auto output_buffer = img_b.pixels();

	while(true)
	{
		auto const delta = run_diffusion_step(output_buffer,
			terraformer::span_2d<float const>{input_buffer},
			diff_params);
		std::swap(input_buffer, output_buffer);

		if(delta < 1.0e-6f)
		{ break; }
	}
	store(input_buffer, "test.exr");
#endif
}
