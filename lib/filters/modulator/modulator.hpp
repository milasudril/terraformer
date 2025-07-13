//@	{"dependencies_extra":[{"ref": "./modulator.o", "rel": "implementation"}]}

#ifndef TERRAFORMER_FILTERS_MODULATOR_HPP
#define TERRAFORMER_FILTERS_MODULATOR_HPP

#include "lib/common/image_registry_view.hpp"

#include "lib/common/span_2d.hpp"
#include "lib/pixel_store/image.hpp"

namespace terraformer::filters
{
	struct modulator_descriptor
	{
		std::u8string input;
		std::u8string modulator;
		float modulation_depth;

		grayscale_image compose_image_from(span_2d_extents output_size, image_registry_view images) const;
	};
}

#endif