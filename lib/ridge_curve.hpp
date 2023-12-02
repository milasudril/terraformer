#ifndef TERRAFORMER_RIDGECURVE_HPP
#define TERRAFORMER_RIDGECURVE_HPP

#include "lib/modules/dimensions.hpp"

namespace terraformer
{
	struct ridge_curve_description
	{
		horizontal_offset initial_value;
		horizontal_amplitude amplitude;
		bool flip_direction{};
		bool invert_displacement{};
		domain_length wavelength;
	};

	std::vector<displacement> generate(ridge_curve_description const& src, float dx);
}

#endif