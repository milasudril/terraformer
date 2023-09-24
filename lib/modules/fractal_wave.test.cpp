//@	{"target":{"name":"fractal_wave.test"}}

#include "./fractal_wave.hpp"

#include <testfwk/testfwk.hpp>
#include <pcg-cpp/include/pcg_random.hpp>

using random_generator = pcg_engines::oneseq_dxsm_128_64;

TESTCASE(fractal_wave_generate)
{
	terraformer::fractal_wave::params const params{
		.amplitude{
			.scaling_factor = std::numbers::phi_v<float>,
			.scaling_noise = std::numbers::phi_v<float>/8.0f
		},
		.wavelength{
			.scaling_factor = std::numbers::phi_v<float>,
			.scaling_noise = std::numbers::phi_v<float>/8.0f
		},
		.phase{
			.offset = 2.0f - std::numbers::phi_v<float>,
			.offset_noise = 1.0f/12.0f
		}
	};

	random_generator rng;
	terraformer::fractal_wave w{rng, params};

	for(size_t k = 0; k != 17; ++k)
	{
		auto const x = static_cast<float>(k)/16.0f;
		printf("%.8g %.8g\n", x, w(x));
	}
}