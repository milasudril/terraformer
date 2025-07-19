//@	{"target": {"name": "modulator.o"}}

#include "./modulator.hpp"

#include "lib/pixel_store/image.hpp"
#include "lib/math_utils/interp.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include "lib/value_maps/log_value_map.hpp"
#include <algorithm>

terraformer::grayscale_image
terraformer::filters::modulator_descriptor::compose_image_from(
	span_2d_extents output_size,
	span_2d<float const> input_image,
	image_registry_view control_images
) const
{
	grayscale_image ret{output_size};
	auto const w_float = static_cast<float>(ret.width());
	auto const h_float = static_cast<float>(ret.height());

	auto const mod_img = control_images.get_image(modulator);
	auto const pixel_count = static_cast<size_t>(mod_img.width())*static_cast<size_t>(mod_img.height());
	auto const range = std::minmax_element(mod_img.data(), mod_img.data() + pixel_count);
	auto const mod_img_min = *range.first;
	auto const mod_img_max = *range.second;
	if(std::abs(mod_img_max - mod_img_min) < 1.0e-6f)
	{ return grayscale_image{input_image}; }
	
	auto const mod_depth = modulation_depth;
	auto const mod_exp = modulator_exponent;

	auto const scale_mod_x = static_cast<float>(mod_img.width())/w_float;
	auto const scale_mod_y = static_cast<float>(mod_img.height())/h_float;
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
			auto const mod_in = interp(mod_img, x_mod, y_mod, clamp_at_boundary{});
			auto const mod = std::pow((mod_in - mod_img_min)/(mod_img_max - mod_img_min),mod_exp) - 1.0f;

			ret(x, y) = in*(mod_depth*mod + 1.0f);
		}
	}

	return ret;
}

void terraformer::filters::modulator_descriptor::bind(descriptor_editor_ref editor)
{
	// TODO: Use a combobox instead
	editor.create_string_input(u8"Modulator",
		modulator,
		descriptor_editor_ref::single_line_text_input_descriptor{
			.textbox_placeholder_string = u8"Lorem ipsum"
		}
	);

	editor.create_float_input(
		u8"Modulator exponent",
		modulator_exponent,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{0.25f, 4.0f, 2.0f}},
			.textbox_placeholder_string = u8"0.123456789",
			.visual_angle_range = std::nullopt
		}
	);

	editor.create_float_input(
		u8"Depth",
		modulation_depth,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::affine_value_map{0.0f, 1.0f}},
			.textbox_placeholder_string = u8"0.123456789",
			.visual_angle_range = std::nullopt
		}
	);

}
