//@	{"target": {"name": "modulator.o"}}

#include "./modulator.hpp"

#include "lib/pixel_store/image.hpp"
#include "lib/math_utils/interp.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include <algorithm>

namespace
{
	terraformer::grayscale_image normalize(terraformer::span_2d<float const> input, float min_out, float max_out)
	{
		terraformer::grayscale_image ret{input.width(), input.height()};
		auto const pixel_count = static_cast<size_t>(input.width())*static_cast<size_t>(input.height());
		auto const range = std::minmax_element(input.data(), input.data() + pixel_count);

		auto const min = *range.first;
		auto const max = *range.second;

		for(uint32_t y = 0; y != ret.height(); ++y)
		{
			for(uint32_t x = 0; x != ret.width(); ++x)
			{
				auto const in = input(x, y);
				ret(x, y) = std::lerp(min_out, max_out, (in - min)/(max - min));
			}
		}

		return ret;
	}
}

terraformer::grayscale_image
terraformer::filters::modulator_descriptor::compose_image_from(
	span_2d_extents output_size,
	image_registry_view images
) const
{
	grayscale_image ret{output_size};
	auto const w_float = static_cast<float>(ret.width());
	auto const h_float = static_cast<float>(ret.height());

	auto const modulator_image = normalize(images.get_image(modulator), -1.0f, 0.0f);
	auto const input_image = images.get_image(input);
	auto const mod_depth = modulation_depth;

	auto const scale_mod_x = static_cast<float>(modulator_image.width())/w_float;
	auto const scale_mod_y = static_cast<float>(modulator_image.height())/h_float;
	auto const scale_input_x = static_cast<float>(input_image.width())/w_float;
	auto const scale_input_y = static_cast<float>(input_image.height())/h_float;

	for(uint32_t y = 0; y != ret.height(); ++y)
	{
		for(uint32_t x = 0; x != ret.width(); ++x)
		{
			auto const x_float = static_cast<float>(x);
			auto const y_float = static_cast<float>(y);
			auto const x_in = (0.5f + x_float)*scale_input_x - 0.5f;
			auto const y_in = (0.5f + y_float)*scale_input_y - 0.5f;
			auto const x_mod = (0.5f + x_float)*scale_mod_x - 0.5f;
			auto const y_mod = (0.5f + y_float)*scale_mod_y - 0.5f;

			auto const in = interp(input_image, x_in, y_in, clamp_at_boundary{});
			auto const mod = interp(modulator_image, x_mod, y_mod, clamp_at_boundary{});

			ret(x, y) = in*(mod_depth*mod + 1.0f);
		}
	}

	return ret;
}