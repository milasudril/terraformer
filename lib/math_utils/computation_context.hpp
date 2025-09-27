#ifndef TERRAFORMER_COMPUTATION_ENGINE_COMPUTATION_CONTEXT_HPP
#define TERRAFORMER_COMPUTATION_ENGINE_COMPUTATION_CONTEXT_HPP

#include "lib/common/move_only_function.hpp"
#include "lib/math_utils/dft_engine.hpp"
#include "lib/execution/thread_pool.hpp"

namespace terraformer
{
	struct computation_context
	{
		// TODO: Would like to have the type of workers without including dft_engine
		thread_pool<move_only_function<void()>> workers;
		class dft_engine dft_engine;
	};
};

#endif