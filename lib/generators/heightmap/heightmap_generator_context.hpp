#ifndef TERRAFORMER_HEIGHTMAP_GENERATOR_CONTEXT_HPP
#define TERRAFORMER_HEIGHTMAP_GENERATOR_CONTEXT_HPP

#include "lib/common/move_only_function.hpp"
#include "lib/execution/thread_pool.hpp"
#include "lib/generators/domain/domain_size.hpp"

#include "lib/math_utils/dft_engine.hpp"

#include <functional>

namespace terraformer
{
	struct heightmap_generator_context
	{
		domain_size_descriptor domain_size;
	//	std::reference_wrapper<class thread_pool<move_only_function<void()>> workers;
	//	std::reference_wrapper<class dft_engine> dft_engine;
	};
};

#endif
