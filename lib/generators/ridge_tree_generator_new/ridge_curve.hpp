//@	{"dependencies_extra": [{"ref": "./ridge_curve.o", "rel": "implementation"}]}

#ifndef TERRAFORMER_RIDGECURVE_HPP
#define TERRAFORMER_RIDGECURVE_HPP

#include "lib/array_classes/single_array.hpp"
#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/utils.hpp"

#include <vector>

namespace terraformer
{
	struct ridge_tree_branch_displacement_description
	{
		float amplitude;
		float wavelength;
		float damping;
		std::optional<float> attack_length = std::nullopt;
	};

	inline constexpr float get_min_pixel_size(ridge_tree_branch_displacement_description const& item)
	{
		return get_min_pixel_size(
			terraformer::wave_descriptor{
				.amplitude = item.amplitude,
				.wavelength = item.wavelength,
				.hf_rolloff = 2.0f
			}
		);
	}

	single_array<float> generate(
		ridge_tree_branch_displacement_description const& src,
		random_generator& rng,
		array_size<float> seg_count,
		float dx,
		float warmup_periods = 0.0f
	);
}

#endif