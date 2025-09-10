#ifndef TERRAFORMER_COMPUTATION_ENGINE_COMPUTATION_CONTEXT_HPP
#define TERRAFORMER_COMPUTATION_ENGINE_COMPUTATION_CONTEXT_HPP

#include "lib/common/move_only_function.hpp"
#include "lib/math_utils/dft_engine.hpp"
#include "lib/execution/thread_pool.hpp"

namespace terraformer
{
	struct computation_context
	{
		thread_pool<move_only_function<void()>> workers;
		class dft_engine dft_engine;
	};
};

#endif