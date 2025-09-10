//@	{"target":{"name":"./heightmap.o"}}

#include "./heightmap.hpp"

#include "lib/array_classes/single_array.hpp"
#include "lib/common/image_registry_view.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/common/value_map.hpp"
#include "lib/math_utils/interp.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/common/string_to_value_map.hpp"

void terraformer::heightmap_generator_channel_strip_descriptor::bind(descriptor_editor_ref editor)
{
	// TODO: Use a combobox instead
	editor.create_string_input(u8"Input",
		input,
		descriptor_editor_ref::single_line_text_input_descriptor{
			.textbox_placeholder_string = u8"Lorem ipsum"
		}
	);

	{
		auto modulation_editor = editor.create_form(
			descriptor_editor_ref::field_descriptor{
				.label = u8"Modulation A"
			},
			descriptor_editor_ref::form_descriptor{}
		);
		modulation_a.bind(modulation_editor);
	}

	{
		auto modulation_editor = editor.create_form(
			descriptor_editor_ref::field_descriptor{
				.label = u8"Modulation B"
			},
			descriptor_editor_ref::form_descriptor{}
		);
		modulation_b.bind(modulation_editor);
	}

	editor.create_float_input(
		descriptor_editor_ref::field_descriptor{
			.label = u8"Gain",
			.expand_layout_cell = true
		},
		gain,
		descriptor_editor_ref::slider_descriptor{
			.value_map = type_erased_value_map{value_maps::affine_value_map{-1.0f, 1.0f}},
			.textbox_placeholder_string = u8"-0.12345",
			.orientation = descriptor_editor_ref::widget_orientation::vertical
		}
	);
}

void terraformer::heightmap_descriptor::bind(descriptor_editor_ref editor)
{
	auto dom_size_editor = editor.create_form(
		descriptor_editor_ref::field_descriptor{
			.label = u8"Domain size",
		},
		descriptor_editor_ref::form_descriptor{}
	);
	domain_size.bind(dom_size_editor);

	auto generators_editor = editor.create_form(
		descriptor_editor_ref::field_descriptor{
			.label = u8"Generators",
		},
		descriptor_editor_ref::form_descriptor{}
	);
	for(auto& item : generators)
	{
		auto editor = generators_editor.create_form(
			descriptor_editor_ref::field_descriptor{
				.label = item.first
			},
			descriptor_editor_ref::form_descriptor{}
		);
		item.second.bind(editor);
	}

	auto channel_strips_editor = editor.create_table(
		descriptor_editor_ref::field_descriptor{
			.label = u8"Mixer",
			.expand_layout_cell = true
		},
		descriptor_editor_ref::table_descriptor{
			.orientation = descriptor_editor_ref::widget_orientation::deduce,
			.field_names = {
				u8"Input",
				u8"Modulation A",
				u8"Modulation B",
				u8"Gain"
			}
		}
	);
	size_t k = 1;
	for(auto& item :channel_strips)
	{
		auto record = channel_strips_editor.add_record(reinterpret_cast<char8_t const*>(std::to_string(k).c_str()));
		item.bind(record);
		record.append_pending_widgets();
		++k;
	}
}

terraformer::grayscale_image terraformer::generate(
	computation_context& comp_ctxt,
	heightmap_descriptor const& descriptor
)
{
	u8string_to_value_map<grayscale_image> inputs;
	uint32_t output_width = 0;
	uint32_t output_height = 0;
	for(auto const& item : descriptor.generators)
	{
		auto img = item.second.generate_heightmap(
			heightmap_generator_context{
				.domain_size = descriptor.domain_size,
				.comp_ctxt = comp_ctxt
			}
		);
		output_height = std::max(img.height(), output_height);
		output_width = std::max(img.width(), output_width);
		inputs.insert(std::pair{item.first, std::move(img)});
	}

	single_array<std::pair<grayscale_image, float>> images_to_mix;
	image_registry_view registry{std::cref(inputs)};
	for(auto& item : descriptor.channel_strips)
	{
		auto& img = std::as_const(inputs).at(item.input);

		auto output_image = img;
		if(!item.modulation_a.modulator.empty())
		{
			output_image = item.modulation_a.compose_image_from(
				span_2d_extents{output_width, output_height},
				output_image.pixels(),
				registry
			);
		}

		if(!item.modulation_b.modulator.empty())
		{
			output_image = item.modulation_b.compose_image_from(
				span_2d_extents{output_width, output_height},
				output_image.pixels(),
				registry
			);
		}

		images_to_mix.push_back(std::pair{std::move(output_image), item.gain});
	}

	terraformer::grayscale_image ret{output_width, output_height};
	for(auto const& item : images_to_mix)
	{ add_resampled(item.first.pixels(), ret.pixels(), item.second); }

	return ret;
}
