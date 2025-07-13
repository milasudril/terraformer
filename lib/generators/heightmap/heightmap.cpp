//@	{"target":{"name":"./heightmap.o"}}

#include "./heightmap.hpp"

#include "lib/array_classes/single_array.hpp"
#include "lib/common/image_registry_view.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/math_utils/interp.hpp"
#include "lib/pixel_store/image.hpp"

void terraformer::heightmap_descriptor::bind(descriptor_editor_ref editor)
{
	auto dom_size_editor = editor.create_form(
		u8"Domain size",
		descriptor_editor_ref::form_descriptor{}
	);
	domain_size.bind(dom_size_editor);

	auto generators_editor = editor.create_form(
		u8"Generators",
		descriptor_editor_ref::form_descriptor{}
	);
	for(auto& item : generators)
	{
		auto editor = generators_editor.create_form(item.first, descriptor_editor_ref::form_descriptor{});
		item.second.input.bind(editor);
	}
}


terraformer::grayscale_image terraformer::generate(heightmap_descriptor const& descriptor)
{
	// NOTE: Must use same ordering as descriptor.generators
	std::map<std::u8string, grayscale_image, std::less<>> inputs;
	uint32_t output_width = 0;
	uint32_t output_height = 0;
	for(auto const& item : descriptor.generators)
	{
		auto img = item.second.input.generate_heightmap(descriptor.domain_size);
		output_height = std::max(img.height(), output_height);
		output_width = std::max(img.width(), output_width);
		inputs.insert(std::pair{item.first, std::move(img)});
	}

	single_array<std::pair<grayscale_image, float>> images_to_mix;
	// TODO: C++23: Use zip view
	auto i = std::begin(std::as_const(inputs));
	auto j = std::begin(descriptor.generators);
	image_registry_view registry{std::cref(inputs)};
	for(;i != std::end(std::as_const(inputs)); ++i, ++j)
	{
		auto& img = i->second;
		if(j->second.modulation.has_value())
		{
			images_to_mix.push_back(
				std::pair{
					j->second.modulation->compose_image_from(
						span_2d_extents{output_width, output_height},
						img.pixels(),
						registry
					),
					j->second.gain
				}
			);
		}
		else
		{ images_to_mix.push_back(std::pair{img, j->second.gain}); }
	}

	terraformer::grayscale_image ret{output_width, output_height};
	for(auto const& item : images_to_mix)
	{ add_resampled(item.first.pixels(), ret.pixels(), item.second); }

	return ret;
}