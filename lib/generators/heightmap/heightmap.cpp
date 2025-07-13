//@	{"target":{"name":"./heightmap.o"}}

#include "./heightmap.hpp"

#include "lib/math_utils/interp.hpp"

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
	auto const plain = descriptor.generators.at(u8"Plain").input.generate_heightmap(descriptor.domain_size);
	auto const rolling_hills = descriptor.generators.at(u8"Rolling hills").input.generate_heightmap(descriptor.domain_size);

	auto const output_width = std::max(plain.width(), rolling_hills.width());
	auto const output_height = std::max(plain.height(), 	rolling_hills.height());;

	grayscale_image ret{output_width, output_height};

	add_resampled(plain.pixels(), ret.pixels(), 1.0f);
	add_resampled(rolling_hills.pixels(), ret.pixels(), 1.0f);

	return ret;
}