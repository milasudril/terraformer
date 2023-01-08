//@	{"target":{"name":"noisy_drift.test"}}

#include "./noisy_drift.hpp"

#include "testfwk/testfwk.hpp"

#include "pcg-cpp/include/pcg_random.hpp"

namespace
{
	using Rng = pcg_engines::oneseq_dxsm_128_64;
}

TESTCASE(terraformer_noisy_drift_expected_norm)
{
	for(size_t k = 0; k != 5; ++k)
	{
		auto const seed = static_cast<unsigned __int128>(time(0));
		terraformer::noisy_drift generator{
			Rng{seed},
			terraformer::noisy_drift<Rng>::params{
				.drift = terraformer::direction{terraformer::geom_space::x{}},
				.noise_amount = static_cast<float>(k)/4.0f
			}
		};

		auto r = 0.0;
		size_t N = 32*65536;
		for(size_t l = 0; l != N; ++l)
		{
			r += norm(generator());
		}
		printf("%.15g\n", r/static_cast<double>(N));
	}
}