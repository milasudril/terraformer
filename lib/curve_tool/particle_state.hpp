#ifndef TERRAFORMER_LIB_PARTICLE_STATE_HPP
#define TERRAFORMER_LIB_PARTICLE_STATE_HPP

#include "lib/common/spaces.hpp"

namespace terraformer
{
	struct particle_state
	{
		displacement v;
		location r;
	};
}

#endif