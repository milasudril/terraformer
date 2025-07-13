//@	{"target":{"name":"./heightmap.o"}}

#include "./heightmap.hpp"

#include "lib/math_utils/interp.hpp"

#include "lib/pixel_store/image_io.hpp"

void terraformer::heightmap_generator_descriptor::bind(descriptor_editor_ref editor)
{
	auto plain_editor = editor.create_form(u8"Plain", descriptor_editor_ref::form_descriptor{});
	plain.bind(plain_editor);

	auto rolling_hills_editor = editor.create_form(
		u8"Rolling hills",
		descriptor_editor_ref::form_descriptor{}
	);
	rolling_hills.bind(rolling_hills_editor);
}

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
	generators.bind(generators_editor);
}


terraformer::grayscale_image terraformer::generate(heightmap_descriptor const& descriptor)
{
	auto const plain = descriptor.generators.plain.generate_heightmap(descriptor.domain_size);
	auto const rolling_hills = descriptor.generators.rolling_hills.generate_heightmap(descriptor.domain_size);

	auto const output_width = std::max(plain.width(), rolling_hills.width());
	auto const output_height = std::max(plain.height(), 	rolling_hills.height());;

	grayscale_image ret{output_width, output_height};

	add_resampled(plain.pixels(), ret.pixels(), 1.0f);
	add_resampled(rolling_hills.pixels(), ret.pixels(), 1.0f);

	return ret;
}