//@	{"target":{"name":"domain_size.o"}}

#include "./domain_size.hpp"
#include "lib/common/value_map.hpp"
#include "lib/descriptor_io/descriptor_editor_ref.hpp"
#include "lib/value_maps/log_value_map.hpp"

void terraformer::domain_size_descriptor::bind(descriptor_editor_ref editor)
{
	editor.create_float_input(
		u8"Width/m",
		width,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{1.0f, 65536.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);

	editor.create_float_input(
		u8"Height/m",
		height,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{1.0f, 65536.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);
}