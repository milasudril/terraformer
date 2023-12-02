//@	{"dependencies_extra": [{"ref": "./ridge_curve.o", "rel": "implementation"}]}

#ifndef TERRAFORMER_RIDGECURVE_HPP
#define TERRAFORMER_RIDGECURVE_HPP

#include "./rng.hpp"
#include "./first_order_hp_filter.hpp"

#include "lib/modules/dimensions.hpp"
#include "lib/common/spaces.hpp"

#include <vector>

namespace terraformer
{
	struct ridge_curve_description
	{
		horizontal_amplitude amplitude;
		domain_length wavelength;
		bool flip_direction{};
		bool invert_displacement{};
	};

	std::vector<displacement> generate(
		ridge_curve_description const& src,
		random_generator& rng,
		size_t seg_count,
		float dx);
}

#endif