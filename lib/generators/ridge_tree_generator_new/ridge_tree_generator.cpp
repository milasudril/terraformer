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
#include "lib/math_utils/butter_lp_2d.hpp"
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
	template<class Shape>
	void  add_circle(terraformer::span_2d<float> output, float x_0, float y_0, float r, Shape&& shape)
	{
		auto const x_min = std::clamp(
			static_cast<int32_t>(x_0 - r + 0.5f),
			0,
			static_cast<int32_t>(output.width())
		);

		auto const y_min = std::clamp(
			static_cast<int32_t>(y_0 - r + 0.5f),
			0,
			static_cast<int32_t>(output.height())
		);

		auto const x_max = std::clamp(
			static_cast<int32_t>(x_0 + r + 0.5f),
			0,
			static_cast<int32_t>(output.width())
		);

		auto const y_max = std::clamp(
			static_cast<int32_t>(y_0 + r + 0.5f),
			0,
			static_cast<int32_t>(output.height())
		);

		for(int32_t y = y_min; y != y_max; ++y)
		{
			for(int32_t x = x_min; x != x_max; ++x)
			{
				auto const x_float = (static_cast<float>(x) + 0.5f - x_0)/r;
				auto const y_float = (static_cast<float>(y) + 0.5f - y_0)/r;
				auto const r = std::sqrt(x_float*x_float + y_float*y_float);
				if( r <= 1.0f)
				{ output(x, y) += shape(1.0f - r); }
			}
		}
	}

	template<class Shape>
	void  replace_circle(terraformer::span_2d<float> output, float x_0, float y_0, float r, Shape&& shape)
	{
		auto const x_min = std::clamp(
			static_cast<int32_t>(x_0 - r + 0.5f),
			0,
			static_cast<int32_t>(output.width())
		);

		auto const y_min = std::clamp(
			static_cast<int32_t>(y_0 - r + 0.5f),
			0,
			static_cast<int32_t>(output.height())
		);

		auto const x_max = std::clamp(
			static_cast<int32_t>(x_0 + r + 0.5f),
			0,
			static_cast<int32_t>(output.width())
		);

		auto const y_max = std::clamp(
			static_cast<int32_t>(y_0 + r + 0.5f),
			0,
			static_cast<int32_t>(output.height())
		);

		for(int32_t y = y_min; y != y_max; ++y)
		{
			for(int32_t x = x_min; x != x_max; ++x)
			{
				auto const x_float = (static_cast<float>(x) + 0.5f - x_0)/r;
				auto const y_float = (static_cast<float>(y) + 0.5f - y_0)/r;
				auto const r = std::sqrt(x_float*x_float + y_float*y_float);
				if( r <= 1.0f)
				{ output(x, y) = shape(1.0f - r); }
			}
		}
	}

	template<class Shape>
	void  max_circle(terraformer::span_2d<float> output, float x_0, float y_0, float r, Shape&& shape)
	{
		auto const x_min = std::clamp(
			static_cast<int32_t>(x_0 - r + 0.5f),
			0,
			static_cast<int32_t>(output.width())
		);

		auto const y_min = std::clamp(
			static_cast<int32_t>(y_0 - r + 0.5f),
			0,
			static_cast<int32_t>(output.height())
		);

		auto const x_max = std::clamp(
			static_cast<int32_t>(x_0 + r + 0.5f),
			0,
			static_cast<int32_t>(output.width())
		);

		auto const y_max = std::clamp(
			static_cast<int32_t>(y_0 + r + 0.5f),
			0,
			static_cast<int32_t>(output.height())
		);

		for(int32_t y = y_min; y != y_max; ++y)
		{
			for(int32_t x = x_min; x != x_max; ++x)
			{
				auto const x_float = (static_cast<float>(x) + 0.5f - x_0)/r;
				auto const y_float = (static_cast<float>(y) + 0.5f - y_0)/r;
				auto const r = std::sqrt(x_float*x_float + y_float*y_float);
				if( r <= 1.0f)
				{ output(x, y) = std::max(output(x,y), shape(1.0f - r)); }
			}
		}
	}

	auto get_parent_ridge_elevation(
		terraformer::displaced_curve::index_type branch_starts_at,
		terraformer::ridge_tree_trunk const& current_trunk,
		terraformer::span<terraformer::ridge_tree_trunk const> all_trunks
	)
	{
		if(current_trunk.parent ==terraformer::ridge_tree_trunk::no_parent)
		{ return 1.0f; }

		auto const& parent = all_trunks[current_trunk.parent];
		auto const& parent_branches = parent.branches;
		auto const curve_lengths = parent_branches.get<3>();
		// This is the correct index
		terraformer::ridge_tree_branch_sequence::index_type const i{current_trunk.parent_curve_index.get()};
		auto const parent_curve_length_tot = curve_lengths[i].back();
		auto const curve_length_at_branch = curve_lengths[i][terraformer::array_index<float>{branch_starts_at.get()}];

		return std::max(1.0f - curve_length_at_branch/parent_curve_length_tot, 0.0f)/
			std::max(1.0f - curve_lengths[i].front()/parent_curve_length_tot, 0.0f);
	}

	auto render_branches_at_current_level(
		terraformer::domain_size_descriptor dom_size,
		terraformer::ridge_tree_descriptor const& params,
		terraformer::ridge_tree_trunk const* i,
		terraformer::ridge_tree_trunk const* i_end,
		terraformer::span<terraformer::ridge_tree_trunk const> all_trunks,
		terraformer::random_generator& rng,
		terraformer::span_2d<float> output_image
	)
	{
		auto const level = i->level;
		auto const pixel_size = get_min_pixel_size(
			params.elevation_profile[level],
			params.horizontal_layout[level]
		);
		auto const w_img_ridge = 2u*std::max(static_cast<uint32_t>(dom_size.width/(2.0f*pixel_size) + 0.5f), 1u);
		auto const h_img_ridge = 2u*std::max(static_cast<uint32_t>(dom_size.height/(2.0f*pixel_size) + 0.5f), 1u);

		terraformer::grayscale_image ridge{w_img_ridge, h_img_ridge};
		auto& ep = params.elevation_profile[level];
		auto const ridge_radius = ep.horizontal_scale_ridge/pixel_size;
		auto const shape_exponent = ep.shape_exponent;
		printf("Rendering level %zu\n", level);
		while(i != i_end)
		{
			if(i->level != level)
			{ break; }

			auto const& branches = i->branches;
			auto const curves = branches.get<0>();
			auto const start_index = branches.get<1>();
			auto const curve_lengths = branches.get<3>();
			for(auto k : branches.element_indices())
			{
				auto const& curve = curves[k];
				if(curve.points().empty())
				{ continue; }

				visit_pixels(curve.points(), pixel_size, [
					ridge = ridge.pixels(),
					&rng,
					ridge_radius,
					loc_prev = terraformer::location{} + (curve.points().front() - terraformer::location{})/pixel_size,
					curve_length = curve_lengths[k].back()/pixel_size,
					travel_distance = 0.0f,
					level,
					shape_exponent,
					ridge_elevation = level<2? 1.0f : get_parent_ridge_elevation(start_index[k], *i, all_trunks)
				](auto loc) mutable{
					auto const x = loc[0];
					auto const y = loc[1];
					auto const d = level == 0? 0.0f :travel_distance/curve_length;
					max_circle(ridge, x, y, ridge_radius, [
						d,
						shape_exponent,
						ridge_elevation
					](float r){
						return ridge_elevation*std::max(1.0f - d, 0.0f)*std::pow(r, shape_exponent);
					});
					travel_distance += distance(loc, loc_prev);
					loc_prev = loc;
				});
			}
			++i;
		}

		{
			auto const minmax = std::minmax_element(
				ridge.pixels().data(),
				ridge.pixels().data() + w_img_ridge*h_img_ridge
			);

			if(*minmax.first < *minmax.second)
			{
				std::transform(
					ridge.pixels().data(),
					ridge.pixels().data() + w_img_ridge*h_img_ridge,
					ridge.pixels().data(),
					[
						min = *minmax.first,
						max = *minmax.second,
						ridge_elevation = ep.ridge_elevation
					](auto val) {
						return ridge_elevation * (val - min)/(max - min);
					}
				);
			}
		}

		terraformer::add_resampled(std::as_const(ridge).pixels(), output_image, 1.0f);
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
	{ i = render_branches_at_current_level(dom_size, params, i, std::end(ridge_tree), ridge_tree, rng, ret.pixels()); }

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
			.value_map = type_erased_value_map{value_maps::log_value_map{1.0f, 8.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);
	editor.create_float_input(
		u8"Horizontal scale (ridge)/m",
		horizontal_scale_ridge,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{128.0f, 65536.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);
	editor.create_float_input(
		u8"Horizontal scale (noise)/m",
		horizontal_scale_noise,
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
					u8"Horizontal scale (ridge)/m",
					u8"Horizontal scale (noise)/m",
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