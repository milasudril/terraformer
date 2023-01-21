//@	{"target":{"name":"curve_gen.o"}}

#include "lib/curve_tool/damped_motion_integrator.hpp"
#include "lib/curve_tool/noisy_drift.hpp"
#include "lib/curve_tool/turn_angle_limiter.hpp"

#include <random>
#include <pcg-cpp/include/pcg_random.hpp>

using random_generator = pcg_engines::oneseq_dxsm_128_64;

int main()
{
	auto const curve_scaling_factor = 6.0f;
	terraformer::location const r_0{0.0f, 512.0f, 0.0f};

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
	while(ps.r[0] < 1024.0f)
	{
		auto const v = drift(rng);
		auto const ps_new = integrator(ps, v);
		auto const r_corrected = limiter(ps_new.r);

		auto const v_corr = 2.0f*(r_corrected - ps.r)/curve_scaling_factor - ps.v;

		ps.v = v_corr;
		ps.r = r_corrected;

		curve.push_back(r_corrected);
	}

	std::ranges::for_each(curve, [](auto const item) {
		printf("%.8e %.8e\n", item[0], item[1]);
	});
}
