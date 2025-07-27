//@	{"target": {"name": "./ridge_tree_generator.o"}}

#include "./ridge_tree_generator.hpp"
#include "./ridge_tree.hpp"

#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/common/value_map.hpp"
#include "lib/generators/domain/domain_size.hpp"
#include "lib/generators/ridge_tree_generator_new/ridge_tree_branch.hpp"
#include "lib/math_utils/butter_bp_2d.hpp"
#include "lib/math_utils/interp.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/value_maps/qurt_value_map.hpp"
#include "lib/value_maps/log_value_map.hpp"
#include "lib/curve_tools/rasterizer.hpp"


#include "lib/curve_tools/dump.hpp"
#include "lib/common/cfile_owner.hpp"

#include <algorithm>
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
		auto const theta = 2.0f*std::numbers::pi_v<float>*params.heading;
		terraformer::location const world_origin{0.5f*dom_size.width, 0.5f*dom_size.height, 0.0f};
		terraformer::location const ridge_origin{params.x_0, params.y_0, 0.0f};

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
		auto const dr = ridge_origin - terraformer::location{0.5f*params.horizontal_layout[0].e2e_distance, 0.0f, 0.0f};
		auto const root_location = world_origin + terraformer::displacement{
			inner_product(dr, ridge_direction),
			-inner_product(dr, dir_ortho),
			0.0f
		};

		std::vector<terraformer::ridge_tree_branch_description> curve_levels;
		for(auto const& item : params.horizontal_layout)
		{
			curve_levels.push_back(
				terraformer::ridge_tree_branch_description{
					.displacement_profile {
						.amplitude = item.displacement.amplitude,
						.wavelength = item.displacement.wavelength,
						.damping = item.displacement.damping
					},
					.growth_params{
						.max_length = item.e2e_distance,
						.min_neighbour_distance = 2.0f*item.displacement.amplitude
					}
				}
			);
		}

		return terraformer::ridge_tree_xy_description{
			.root_location = root_location,
			.trunk_direction = ridge_direction,
			.curve_levels = std::move(curve_levels)
		};
	}

	auto render_branches_at_current_level(
		terraformer::domain_size_descriptor dom_size,
		terraformer::ridge_tree_descriptor const& params,
		terraformer::ridge_tree_trunk const* i,
		terraformer::ridge_tree_trunk const* i_end,
		terraformer::random_generator& rng,
		terraformer::span_2d<float> output_image
	)
	{
		auto const level = i->level;
		auto const pixel_size = get_min_pixel_size(
			params.elevation_profile[level],
			params.horizontal_layout[level]
		);
		auto const w_img = 2u*std::max(static_cast<uint32_t>(dom_size.width/(2.0f*pixel_size) + 0.5f), 1u);
		auto const h_img = 2u*std::max(static_cast<uint32_t>(dom_size.height/(2.0f*pixel_size) + 0.5f), 1u);
		printf("Image size for level %zu: %u x %u\n", level, w_img, h_img);

		terraformer::grayscale_image tmp{w_img, h_img};
		while(i != i_end)
		{
			if(i->level != level)
			{ break; }

			for(auto const& curve : i->branches.get<0>())
			{
				visit_pixels(curve.points(), pixel_size, [result = tmp.pixels(), &rng](float x, float y, auto&&...){
					std::uniform_real_distribution pixel_value{0.0f, 1.0f};
					auto const target_x = static_cast<int32_t>(x + 0.5f);
					auto const target_y = static_cast<int32_t>(y + 0.5f);
					if(
						(target_x >= 0 && static_cast<uint32_t>(target_x) < result.width()) &&
						(target_y >= 0 && static_cast<uint32_t>(target_y) < result.height())
					)
					{ result(target_x, target_y) = pixel_value(rng); }
				});
			}
			++i;
		}

		auto& ep = params.elevation_profile[level];
		tmp = apply(
			terraformer::butter_bp_2d_descriptor{
				.f_x = dom_size.width/ep.horizontal_scale,
				.f_y = dom_size.height/ep.horizontal_scale,
				.lf_rolloff = ep.lf_rolloff,
				.hf_rolloff = ep.hf_rolloff,
				.y_direction = 0.0f
			},
			std::as_const(tmp).pixels()
		);

		terraformer::add_resampled(std::as_const(tmp).pixels(), output_image, 1.0f);
		return i;
	}
}

float terraformer::get_min_pixel_size(terraformer::ridge_tree_descriptor const& params)
{
	auto const min_layout = std::ranges::min_element(
		params.horizontal_layout,
		[](auto const& a, auto const& b)
		{ return get_min_pixel_size(a) < get_min_pixel_size(b); }
	);

	auto const min_elevation_profile = std::ranges::min_element(
		params.elevation_profile,
		[](auto const& a, auto const& b)
		{ return get_min_pixel_size(a) < get_min_pixel_size(b); }
	);

	return get_min_pixel_size(*min_layout, *min_elevation_profile);
}

terraformer::grayscale_image
terraformer::generate(domain_size_descriptor dom_size, ridge_tree_descriptor const& params)
{
	auto const rng_seed = std::bit_cast<terraformer::rng_seed_type>(params.rng_seed);
	terraformer::random_generator rng{rng_seed};

	auto const ridge_tree = generate(collect_ridge_tree_xy_params(dom_size, params), rng);
	if(ridge_tree.size().get() == 0)
	{ return terraformer::grayscale_image{16, 16}; }

	auto const global_pixel_size = get_min_pixel_size(params);
	auto const w_img = 2u*std::max(static_cast<uint32_t>(dom_size.width/(2.0f*global_pixel_size) + 0.5f), 1u);
	auto const h_img = 2u*std::max(static_cast<uint32_t>(dom_size.height/(2.0f*global_pixel_size) + 0.5f), 1u);
	grayscale_image ret{w_img, h_img};

	auto i = std::begin(ridge_tree);
	while(i != std::end(ridge_tree))
	{
		i = render_branches_at_current_level(dom_size, params, i, std::end(ridge_tree), rng, ret.pixels());
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
			.value_map = type_erased_value_map{value_maps::log_value_map{128.0f, 65536.0f, 2.0f}},
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

void terraformer::ridge_tree_horz_layout_descriptor::bind(descriptor_editor_ref editor)
{
	editor.create_float_input(
		u8"E2E distance/m",
		e2e_distance,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{1.0f, 65536.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);
	displacement.bind(editor);
}

void terraformer::ridge_tree_elevation_profile_descriptor::bind(descriptor_editor_ref editor)
{
	editor.create_float_input(
		u8"Ridge elevation/m",
		ridge_elevation,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{1.0f, 8192.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);
	editor.create_float_input(
		u8"Noise amplitude/m",
		noise_amplitude,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{1.0f, 8192.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);
	editor.create_float_input(
		u8"LF roll-off",
		lf_rolloff,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{1.0f, 8.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);
	editor.create_float_input(
		u8"HF roll-off",
		hf_rolloff,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{2.0f, 8.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);
	editor.create_float_input(
		u8"Horizontal scale/m",
		horizontal_scale,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{128.0f, 65536.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);
	editor.create_float_input(
		u8"Shape exponent",
		shape_exponent,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{0.25f, 4.0f, 2.0f}},
			.textbox_placeholder_string = u8"0.123456789",
			.visual_angle_range = std::nullopt
		}
	);
}

void terraformer::ridge_tree_descriptor::bind(descriptor_editor_ref editor)
{
	editor.create_rng_seed_input(u8"Seed", rng_seed);

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

	{
		auto horz_layout_table = editor.create_table(
			descriptor_editor_ref::field_descriptor{
				.label = u8"Horizontal layout"
			},
			descriptor_editor_ref::table_descriptor{
				.orientation = descriptor_editor_ref::widget_orientation::horizontal,
				.field_names{
					u8"E2E distance/m",
					u8"Amplitude/m",
					u8"Wavelength/m",
					u8"Damping"
				}
			}
		);
		size_t k = 0;
		for(auto& item : horizontal_layout)
		{
			auto record = horz_layout_table.add_record(reinterpret_cast<char8_t const*>(std::to_string(k).c_str()));
			item.bind(record);
			record.append_pending_widgets();
			++k;
		}
	}

	{
		auto elev_profile_table = editor.create_table(
			descriptor_editor_ref::field_descriptor{
				.label = u8"Elevation profile"
			},
			descriptor_editor_ref::table_descriptor{
				.orientation = descriptor_editor_ref::widget_orientation::horizontal,
				.field_names{
					u8"Ridge elevation/m",
					u8"Noise amplitude/m",
					u8"LF roll-off",
					u8"HF roll-off",
					u8"Horizontal scale/m",
					u8"Shape exponent"
				}
			}
		);
		size_t k = 0;
		for(auto& item : elevation_profile)
		{
			auto record = elev_profile_table.add_record(reinterpret_cast<char8_t const*>(std::to_string(k).c_str()));
			item.bind(record);
			record.append_pending_widgets();
			++k;
		}
	}
}