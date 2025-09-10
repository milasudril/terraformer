#ifndef TERRAFORMER_HEIGHTMAP_GENERATOR_CONTEXT_HPP
#define TERRAFORMER_HEIGHTMAP_GENERATOR_CONTEXT_HPP

#include "lib/computation_engine/computation_context.hpp"
#include "lib/generators/domain/domain_size.hpp"

#include "lib/math_utils/dft_engine.hpp"

#include <functional>

namespace terraformer
{
	struct heightmap_generator_context
	{
		domain_size_descriptor domain_size;
		std::reference_wrapper<computation_context> comp_ctxt;
	};
};

#endif
