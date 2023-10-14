//@	{"target":{"name":"fractal_wave.test"}}

#include "./fractal_wave.hpp"

#include <testfwk/testfwk.hpp>
#include <pcg-cpp/include/pcg_random.hpp>

using random_generator = pcg_engines::oneseq_dxsm_128_64;

TESTCASE(fractal_wave_generate)
{
	terraformer::fractal_wave::params const params{
		.amplitude{
			.factor = terraformer::scaling_factor{2.0f},
			.scaling_noise = terraformer::noise_amplitude{0.0f}
		},
		.wavelength{
			.factor = terraformer::scaling_factor{1.0f},
			.scaling_noise = terraformer::noise_amplitude{0.0f}
		},
		.phase{
			.offset = terraformer::phase_offset{0.0f},
			.offset_noise = terraformer::noise_amplitude{0.0f}
		}
	};

	random_generator rng;
	terraformer::fractal_wave w{rng, params};

	for(size_t k = 0; k != 17; ++k)
	{
		auto const x = static_cast<float>(k)/16.0f;
		printf("%.8g    %.8g\n", x, w(x));
	}
}

TESTCASE(fractal_wave_generate_no_amp_scaling)
{
	terraformer::fractal_wave::params const params{
		.amplitude{
			.factor = terraformer::scaling_factor{1.0f},
			.scaling_noise = terraformer::noise_amplitude{0.0f}
		},
		.wavelength{
			.factor = terraformer::scaling_factor{1.0f},
			.scaling_noise = terraformer::noise_amplitude{0.0f}
		},
		.phase{
			.offset = terraformer::phase_offset{0.0f},
			.offset_noise = terraformer::noise_amplitude{0.0f}
		}
	};

	random_generator rng;
	terraformer::fractal_wave w{rng, params};

	for(size_t k = 0; k != 17; ++k)
	{
		auto const x = static_cast<float>(k)/16.0f;
		printf("%.8g    %.8g\n", x, w(x));
	}
}