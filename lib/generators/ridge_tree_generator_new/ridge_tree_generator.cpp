//@	{"target": {"name": "./ridge_tree_generator.o"}}

#include "./ridge_tree_generator.hpp"
#include "./ridge_tree.hpp"

#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/value_map.hpp"
#include "lib/value_maps/qurt_value_map.hpp"
#include "lib/value_maps/log_value_map.hpp"
#include "lib/curve_tools/rasterizer.hpp"


#include "lib/curve_tools/dump.hpp"
#include "lib/common/cfile_owner.hpp"

#include <cassert>
#include <numbers>
#include <random>

namespace
{
	terraformer::ridge_tree_xy_description
	collect_ridge_tree_xy_params(
		terraformer::domain_size_descriptor dom_size,
		terraformer::ridge_tree_descriptor const& params
	)
	{
		auto const theta = 2.0f*std::numbers::pi_v<float>*params.trunk.heading;
		terraformer::location const world_origin{0.5f*dom_size.width, 0.5f*dom_size.height, 0.0f};
		terraformer::location const ridge_origin{params.trunk.x_0, params.trunk.y_0, 0.0f};

		auto const sin_theta = std::sin(theta);
		auto const cos_theta = std::cos(theta);
		terraformer::direction const ridge_direction{
			terraformer::displacement{
				sin_theta,
				-cos_theta,
				0.0f
			}
		};
		terraformer::direction const dir_ortho{
			terraformer::displacement{
				cos_theta,
				sin_theta,
				0.0f
			}
		};
		auto const dr = ridge_origin - terraformer::location{0.5f*params.trunk.e2e_distance, 0.0f, 0.0f};
		auto const root_location = world_origin + terraformer::displacement{
			inner_product(dr, ridge_direction),
			-inner_product(dr, dir_ortho),
			0.0f
		};

		return terraformer::ridge_tree_xy_description{
			.root_location = root_location,
			.trunk_direction = ridge_direction,
			.curve_levels = std::vector{
				terraformer::ridge_tree_branch_description{
					.displacement_profile {
						.amplitude = params.trunk.horz_displacement.amplitude,
						.wavelength = params.trunk.horz_displacement.wavelength,
						.damping = params.trunk.horz_displacement.damping
					},
					.growth_params{
						.max_length = params.trunk.e2e_distance,
						.min_neighbour_distance = params.trunk.e2e_distance
					}
				}
			}
		};
	}
}

terraformer::grayscale_image
terraformer::generate(domain_size_descriptor dom_size, ridge_tree_descriptor const& params)
{
	auto const rng_seed = std::bit_cast<terraformer::rng_seed_type>(params.rng_seed);
	terraformer::random_generator rng{rng_seed};

	auto const T_0 = params.trunk.horz_displacement.wavelength;
	auto const pixel_size = T_0/128.0f;  // Allow 6 octaves within 2^-12
	auto const w_img = std::max(static_cast<uint32_t>(dom_size.width/pixel_size + 0.5f), 1u);
	auto const h_img = std::max(static_cast<uint32_t>(dom_size.height/pixel_size + 0.5f), 1u);

	grayscale_image ret{w_img, h_img};

	auto res = generate(collect_ridge_tree_xy_params(dom_size, params), rng, pixel_size);

	for(auto const& item : res)
	{
		for(auto const& curve : item.branches.get<0>())
		{
			visit_pixels(curve.points(), pixel_size, [result = ret.pixels()](float x, float y, auto&&...){
				auto const target_x = static_cast<int32_t>(x + 0.5f);
				auto const target_y = static_cast<int32_t>(y + 0.5f);
				if(
					(target_x >= 0 && static_cast<uint32_t>(target_x) < result.width()) &&
					(target_y >= 0 && static_cast<uint32_t>(target_y) < result.height())
				)
				{ result(target_x, target_y) = 3600.0f; }
			});
		}
	}



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
			.value_map = type_erased_value_map{value_maps::log_value_map{1024.0f, 65536.0f, 2.0f}},
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