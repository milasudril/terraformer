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
	auto const seed = static_cast<unsigned __int128>(time(0));
	Rng rng{seed};

	for(size_t k = 0; k != 5; ++k)
	{
		for(size_t l = 0; l != 5; ++l)
		{
			geosimd::rotation_angle const expected_theta{geosimd::turns{static_cast<double>(k)/4.0}};

			terraformer::noisy_drift generator{
				terraformer::noisy_drift::params{
					.drift = expected_theta,
					.noise_amount = static_cast<float>(l)/4.0f
				}
			};

			auto cs = cossin(expected_theta);
			terraformer::direction const expected_dir{terraformer::displacement{cs.cos, cs.sin, 0.0f}};

			auto r = 0.0;
			auto theta = geosimd::turn_angle{0};
			size_t N = 16*65536;
			for(size_t m = 0; m != N; ++m)
			{
				auto const v = generator(rng);
				r += norm(v);
				theta += angular_difference(terraformer::direction{v}, expected_dir);
			}
			auto const E_r = r/static_cast<double>(N);
			auto const E_theta = to_turns(theta).value/static_cast<double>(N);
			EXPECT_LT(std::abs(E_r - 1.0), 1.0/1024.0);
			EXPECT_LT(std::abs(E_theta), 1.0/1024.0);
		}
	}
}