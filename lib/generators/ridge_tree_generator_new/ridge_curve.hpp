//@	{"dependencies_extra": [{"ref": "./ridge_curve.o", "rel": "implementation"}]}

#ifndef TERRAFORMER_RIDGECURVE_HPP
#define TERRAFORMER_RIDGECURVE_HPP

#include "lib/array_classes/single_array.hpp"
#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"

#include <vector>

namespace terraformer
{
	struct ridge_tree_branch_displacement_description
	{
		float amplitude;
		float wavelength;
		float damping;
	};

	single_array<float> generate(
		ridge_tree_branch_displacement_description const& src,
		random_generator& rng,
		array_size<float> seg_count,
		float dx,
		float warmup_periods = 0.0f
	);
}

#endif