//@	{"target": {"name": "./ridge_tree_generator.o"}}

#include "./ridge_tree_generator.hpp"

#include "lib/common/rng.hpp"
#include "lib/common/value_map.hpp"
#include "lib/value_maps/qurt_value_map.hpp"
#include "lib/value_maps/log_value_map.hpp"

#include <cassert>
#include <random>

terraformer::grayscale_image
terraformer::generate(domain_size_descriptor, ridge_tree_descriptor const&)
{
	terraformer::grayscale_image ret{1, 1};
	return ret;
}

terraformer::grayscale_image
terraformer::ridge_tree_descriptor::generate_heightmap(domain_size_descriptor size) const
{ return generate(size, *this); }

void terraformer::ridge_tree_branch_horz_displacement_descriptor::bind(descriptor_editor_ref editor)
{
	editor.create_float_input(
		u8"Amplitude/m",
		amplitude,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{1.0f, 65536.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);

	editor.create_float_input(
		u8"Wavelength/m",
		wavelength,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{1.0f, 65536.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);

	editor.create_float_input(
		u8"Damping",
		descriptor_editor_ref::assigner<float>{damping},
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::affine_value_map{0.0f, 1.0f}},
			.textbox_placeholder_string = u8"0.123456789",
			.visual_angle_range = std::nullopt
		}
	);
}

void terraformer::ridge_tree_trunk_descriptor::bind(descriptor_editor_ref editor)
{
	editor.create_float_input(
		u8"Center loc x/m",
		x_0,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::qurt_value_map{32767.0f}},
			.textbox_placeholder_string = u8"-9999.9999",
			.visual_angle_range = std::nullopt
		}
	);

	editor.create_float_input(
		u8"Center loc y/m",
		y_0,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::qurt_value_map{32767.0f}},
			.textbox_placeholder_string = u8"-9999.9999",
			.visual_angle_range = std::nullopt
		}
	);

	editor.create_float_input(
		u8"E2E distance/m",
		e2e_distance,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{1.0f, 65536.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);

	editor.create_float_input(
		u8"Heading",
		heading,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::affine_value_map{-0.25f, 0.25f}},
			.textbox_placeholder_string = u8"-0.123456789",
			.visual_angle_range = closed_closed_interval<geosimd::turn_angle>{
				geosimd::turns{1.0/4.0},
				geosimd::turns{3.0/4.0}
			}
		}
	);

	auto displacement_form = editor.create_form(
		descriptor_editor_ref::field_descriptor{
			.label = u8"Horz displacement"
		},
		descriptor_editor_ref::form_descriptor{
			.orientation = descriptor_editor_ref::widget_orientation::vertical
		}
	);
	horz_displacement.bind(displacement_form);
}


void terraformer::ridge_tree_descriptor::bind(descriptor_editor_ref editor)
{
	editor.create_rng_seed_input(u8"Seed", rng_seed);
	auto trunk_form = editor.create_form(
		descriptor_editor_ref::field_descriptor{
			.label = u8"Trunk"
		},
		descriptor_editor_ref::form_descriptor{
			.orientation = descriptor_editor_ref::widget_orientation::vertical
		}
	);
	trunk.bind(trunk_form);
}