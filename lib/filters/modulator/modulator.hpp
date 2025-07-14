//@	{"dependencies_extra":[{"ref": "./modulator.o", "rel": "implementation"}]}

#ifndef TERRAFORMER_FILTERS_MODULATOR_HPP
#define TERRAFORMER_FILTERS_MODULATOR_HPP

#include "lib/common/image_registry_view.hpp"

#include "lib/common/span_2d.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/descriptor_io/descriptor_editor_ref.hpp"

namespace terraformer::filters
{
	struct modulator_descriptor
	{
		std::u8string modulator;
		float modulator_exponent = 1.0f;
		float modulation_depth = 0.5f;

		grayscale_image compose_image_from(
			span_2d_extents output_size,
			span_2d<float const> input,
			image_registry_view control_images
		) const;

		void bind(descriptor_editor_ref);
	};
}

#endif