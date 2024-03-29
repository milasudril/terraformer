//@	{"dependencies_extra": [{"ref": "./ridge_curve.o", "rel": "implementation"}]}

#ifndef TERRAFORMER_RIDGECURVE_HPP
#define TERRAFORMER_RIDGECURVE_HPP

#include "lib/modules/dimensions.hpp"
#include "lib/array_classes/single_array.hpp"
#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"

#include <vector>

namespace terraformer
{
	struct ridge_tree_branch_displacement_description
	{
		horizontal_amplitude amplitude;
		domain_length wavelength;
		float damping;  // TODO: Should be within ]0, 1[
	};

	single_array<float> generate(
		ridge_tree_branch_displacement_description const& src,
		random_generator& rng,
		array_size<float> seg_count,
		float dx,
		float warmup_periods = 0.0f);
}

#endif