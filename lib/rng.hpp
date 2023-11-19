#ifndef TERRAFORMER_RNG_HPP
#define TERRAFORMER_RNG_HPP

#include <pcg-cpp/include/pcg_random.hpp>

namespace terraformer
{
	using random_generator = pcg_engines::oneseq_dxsm_128_64;
	using rng_seed_type = __int128 unsigned;
}

#endif