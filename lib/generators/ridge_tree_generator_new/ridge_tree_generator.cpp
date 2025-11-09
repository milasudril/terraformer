//@	{"target": {"name": "./ridge_tree_generator.o"}}

#include "./ridge_tree_generator.hpp"
#include "./ridge_tree.hpp"

#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/common/value_map.hpp"
#include "lib/descriptor_io/descriptor_editor_ref.hpp"
#include "lib/generators/domain/domain_size.hpp"
#include "lib/generators/heightmap/heightmap_generator_context.hpp"
#include "lib/generators/ridge_tree_generator_new/ridge_curve.hpp"
#include "lib/generators/ridge_tree_generator_new/ridge_tree_branch.hpp"
#include "lib/generators/ridge_tree_generator_new/ridge_tree_branch_seed_sequence.hpp"
#include "lib/math_utils/butter_bp_2d.hpp"
#include "lib/math_utils/butter_lp_2d.hpp"
#include "lib/math_utils/computation_context.hpp"
#include "lib/math_utils/cubic_spline.hpp"
#include "lib/math_utils/interp.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/value_maps/qurt_value_map.hpp"
#include "lib/value_maps/log_value_map.hpp"
#include "lib/curve_tools/rasterizer.hpp"
#include "lib/curve_tools/dump.hpp"

#include <algorithm>
#include <cassert>
#include <geosimd/unit_vector.hpp>
#include <numbers>
#include <random>
#include <cfenv>

// TODO:
//
// To add:
//
// * Branch trimming within begin/endpoint branches. Currently there is no check because of left/right
//   separation.
//
// * Avoid branch early. Looks strange, especially at the root curve
//
// * Perhaps add longitudinal modulation
//
// * Multi-threading where possible
//

namespace
{
	template<class Shape>
	void  max_circle(
		terraformer::span_2d<float> output,
		terraformer::location loc,
		terraformer::direction tangent,
		terraformer::direction normal,
		float r_0,
		Shape&& shape,
		float norm
	)
	{
		auto const x_0 = loc[0];
		auto const y_0 = loc[1];
		auto const x_min = std::clamp(
			static_cast<int32_t>(x_0 - r_0 + 0.5f),
			0,
			static_cast<int32_t>(output.width())
		);

		auto const y_min = std::clamp(
			static_cast<int32_t>(y_0 - r_0 + 0.5f),
			0,
			static_cast<int32_t>(output.height())
		);

		auto const x_max = std::clamp(
			static_cast<int32_t>(x_0 + r_0 + 0.5f),
			0,
			static_cast<int32_t>(output.width())
		);

		auto const y_max = std::clamp(
			static_cast<int32_t>(y_0 + r_0 + 0.5f),
			0,
			static_cast<int32_t>(output.height())
		);

		for(int32_t y = y_min; y != y_max; ++y)
		{
			for(int32_t x = x_min; x != x_max; ++x)
			{
				auto const v = (
					  (terraformer::location{static_cast<float>(x), static_cast<float>(y), 0.0f} - loc)
					+ terraformer::displacement{0.5f, 0.5f, 0.0f}
				)/r_0;

				auto const xi = inner_product(v, tangent);
				auto const eta = inner_product(v, normal);
				auto const r = std::pow(
					  std::pow(std::abs(xi), norm) + std::pow(std::abs(eta), norm),
					1.0f/norm
				);

				if( r <= 1.0f)
				{ output(x, y) = std::max(output(x,y), shape(1.0f - r)); }
			}
		}
	}
}

float terraformer::get_min_pixel_size(terraformer::ridge_tree_descriptor const& params)
{
	auto const min_layout = std::ranges::min_element(
		params.horz_displacements,
		[](auto const& a, auto const& b)
		{ return get_min_pixel_size(a) < get_min_pixel_size(b); }
	);

	auto const min_height_profile = std::ranges::min_element(
		params.height_profile,
		[](auto const& a, auto const& b)
		{ return get_min_pixel_size(a) < get_min_pixel_size(b); }
	);

	return get_min_pixel_size(*min_layout, *min_height_profile);
}


void terraformer::fill_curve(
	span_2d<float> pixels,
	span_2d<float const> pixels_in,
	ridge_tree_trunk const& trunk,
	ridge_tree_ridge_height_profile const& elev_profile,
	float pixel_size
)
{
	auto const elems = trunk.branches.element_indices();
	auto const attribs = trunk.branches.attributes();
	auto const curves = attribs.get<0>();
	auto const rolloff_exponent = elev_profile.rolloff_exponent;
	auto const height_is_relative = elev_profile.height_is_relative;
	auto const height_in = elev_profile.height;
	auto const ridge_radius = elev_profile.relative_half_thickness/pixel_size;

	for(auto k : elems)
	{
		auto const& curve = curves[k];
		if(curve.points().empty())
		{ continue; }

		auto const start_loc = (curve.points().front() - location{})/pixel_size;
		auto const height =
			height_in*(
				height_is_relative?
					interp(pixels_in, start_loc[0], start_loc[1], clamp_at_boundary{}) :
					1.0f
			);

		if(distance(curve.points().back(), curve.points().front()) <= ridge_radius*height)
		{ continue; }

		visit_pixels(
			curve.points(),
			pixel_size,
			[
				pixels,
				ridge_radius,
				rolloff_exponent,
				height
			](location loc, direction tangent, direction normal) mutable {
				max_circle(
					pixels,
					loc,
					tangent,
					normal,
					ridge_radius*height,
					[
						rolloff_exponent,
						height
					](float r){
						return height*std::pow(r, rolloff_exponent);
					},
					2.0f
				);
			}
		);
	}
}

namespace
{
	void add(terraformer::span_2d<float> output, terraformer::span_2d<float const> input)
	{
		for(uint32_t y = 0; y != output.height(); ++y)
		{
			for(uint32_t x = 0; x != output.width(); ++x)
			{ output(x, y) += input(x, y); }
		}
	}

	template<class T>
	void set_collision_margins(T const& stem, float height_factor)
	{
		auto initial_heights = stem.template get<1>();
		auto collision_margins = stem.template get<4>();
		for(auto k : stem.element_indices())
		{
			auto const end_radius = height_factor*initial_heights[k];
			collision_margins[k] = end_radius;
		}
	}

	struct noise_params
	{
		float wavelength;
		float lf_rolloff;
		float hf_rolloff;
	};

	void make_filtered_noise(
		terraformer::span_2d<float> output,
		noise_params const& params,
		terraformer::heightmap_generator_context const& ctxt,
		terraformer::random_generator& rng
	)
	{
		std::uniform_real_distribution noise_dist{0.0f, 1.0f};
		for(uint32_t y = 0; y != output.height(); ++y)
		{
			for(uint32_t x = 0; x != output.width(); ++x)
			{ output(x, y) = noise_dist(rng); }
		}

		auto const filtered_noise = terraformer::apply(
			terraformer::butter_bp_2d_descriptor{
				.f_x = 2.0f*ctxt.domain_size.width/params.wavelength,
				.f_y = 2.0f*ctxt.domain_size.height/params.wavelength,
				.lf_rolloff = params.lf_rolloff,
				.hf_rolloff = params.hf_rolloff,
				.y_direction = 0.0f
			},
			output,
			ctxt.comp_ctxt
		);

		auto const minmax = std::ranges::minmax_element(filtered_noise.pixels());
		auto const min = *minmax.min;
		auto const max = *minmax.max;

		if(min < max)
		{
			std::transform(
				std::begin(filtered_noise.pixels()),
				std::end(filtered_noise.pixels()),
				std::begin(output),
				[
					min,
					max
				](auto val) {
					return (val - min)/(max - min);
				}
			);
		}
	}

	void modulate_with_noise(
		terraformer::span_2d<float> image,
		noise_params const& params,
		float noise_amplitude,
		terraformer::heightmap_generator_context const& ctxt,
		terraformer::random_generator& rng
	)
	{
		auto noise = create_with_same_size(image);
		make_filtered_noise(noise.pixels(), params, ctxt, rng);
		auto const maxval = *std::ranges::max_element(image);
		for(uint32_t y = 0; y != image.height(); ++y)
		{
			for(uint32_t x = 0; x != image.width(); ++x)
			{
				auto const current_val = image(x, y);
				auto const noise_factor = current_val/maxval;
				auto const input_noise_val = noise(x, y);
				auto const noise_val = 2.0f*noise_amplitude*input_noise_val;
				image(x, y) = current_val + noise_val*noise_factor;
			}
		}
	}
}

terraformer::grayscale_image
terraformer::generate(terraformer::heightmap_generator_context const& ctxt, ridge_tree_descriptor const& params)
{
	auto const rng_seed = std::bit_cast<terraformer::rng_seed_type>(params.rng_seed);
	terraformer::random_generator rng{rng_seed};

	auto const dom_size = ctxt.domain_size;
	auto const global_pixel_size = get_min_pixel_size(params);
	auto const w_img = 2u*std::max(static_cast<uint32_t>(dom_size.width/(2.0f*global_pixel_size) + 0.5f), 1u);
	auto const h_img = 2u*std::max(static_cast<uint32_t>(dom_size.height/(2.0f*global_pixel_size) + 0.5f), 1u);
	grayscale_image ret{w_img, h_img};

	single_array<ridge_tree_trunk> trunks;
	trunks.push_back(generate_trunk(dom_size, params.trunk.curve, params.horz_displacements.front(), rng));

	auto const& trunk_height_profile = params.height_profile[0];
	auto const trunk_ridge_height = params.trunk.ridge_height;
	fill_curve(
		ret,
		span_2d<float const>{},
		trunks.back(),
		ridge_tree_ridge_height_profile{
			.height = params.trunk.ridge_height,
			.height_is_relative = false,
			.relative_half_thickness = trunk_height_profile.rel_half_thickness
				*(trunk_height_profile.noise_amplitude + trunk_ridge_height)/trunk_ridge_height,
			.rolloff_exponent = trunk_height_profile.rolloff_exponent
		},
		global_pixel_size
	);

	auto trace_input = ret;
	modulate_with_noise(
		ret,
		noise_params{
			.wavelength = trunk_height_profile.noise_wavelength,
			.lf_rolloff = trunk_height_profile.noise_lf_rolloff,
			.hf_rolloff = trunk_height_profile.noise_hf_rolloff
		},
		trunk_height_profile.noise_amplitude,
		ctxt,
		rng
	);

	auto current_trunk_index = trunks.element_indices().front();
	auto const& branch_growth_params = params.branch_growth_params;
	auto const& displacement_profiles = params.horz_displacements;
	while(true)
	{
		if(current_trunk_index == std::size(trunks))
		{ break; }

		auto& current_trunk = trunks[current_trunk_index];
		auto const next_level_index = current_trunk.level + 1;
		if(next_level_index == std::size(branch_growth_params) + 1)
		{
			++current_trunk_index;
			continue;
		}

		auto next_level_seeds = collect_ridge_tree_branch_seeds(
			std::as_const(current_trunk.branches).get<0>(),
				ridge_tree_branch_seed_collection_descriptor{
				.start_branches = next_level_index == 1?
					params.trunk.starting_point_branches:
					ridge_tree_brach_seed_sequence_boundary_point_descriptor{
						.branch_count = 0,
						.spread_angle = geosimd::turns{0.5f}
					}
				,
				.end_brancehs = params.endpoint_branches[next_level_index - 1]
			}
		);

		auto const& horz_displacement = displacement_profiles[next_level_index];
		auto const& growth_params = branch_growth_params[next_level_index - 1];
		auto const& height_profile = params.height_profile[next_level_index];

		auto next_level = generate_branches(
			next_level_seeds,
			trace_input.pixels(),
			global_pixel_size,
			ridge_tree_branch_displacement_description{
				.amplitude = horz_displacement.amplitude,
				.wavelength = horz_displacement.wavelength,
				.damping = horz_displacement.damping
			},
			rng,
			ridge_tree_branch_growth_description{
				.max_length = growth_params.e2e_distance
			}
		);

		for(auto& stems : next_level)
		{
			auto const height_factor = growth_params.begin_height*height_profile.rel_half_thickness;
			set_collision_margins(stems.left.attributes(), height_factor);
			set_collision_margins(stems.right.attributes(), height_factor);
		}

		trim_at_intersct(next_level);

		ridge_tree_ridge_height_profile const current_height_profile{
			.height = growth_params.begin_height,
			.height_is_relative = true,
			.relative_half_thickness = height_profile.rel_half_thickness,
			.rolloff_exponent = height_profile.rolloff_exponent,
		};

		grayscale_image tmp{w_img, h_img};
		for(auto& stem: next_level)
		{
			if(!stem.left.empty())
			{
				trunks.push_back(
					ridge_tree_trunk{
						.level = next_level_index,
						.branches = std::move(stem.left),
						.parent = current_trunk_index,
						.parent_curve_index = stem.parent_curve_index,
						.side = ridge_tree_trunk::side::left
					}
				);

				fill_curve(tmp, ret.pixels(), trunks.back(), current_height_profile, global_pixel_size);
			}

			if(!stem.right.empty())
			{
				trunks.push_back(
					ridge_tree_trunk{
						.level = next_level_index,
						.branches = std::move(stem.right),
						.parent = current_trunk_index,
						.parent_curve_index = stem.parent_curve_index,
						.side = ridge_tree_trunk::side::right
					}
				);

				fill_curve(tmp, ret.pixels(), trunks.back(), current_height_profile, global_pixel_size);
			}
		}
		add(trace_input.pixels(), std::as_const(tmp).pixels());
		modulate_with_noise(
			tmp,
			noise_params{
				.wavelength = height_profile.noise_wavelength,
				.lf_rolloff = height_profile.noise_lf_rolloff,
				.hf_rolloff = height_profile.noise_hf_rolloff
			},
			height_profile.noise_amplitude,
			ctxt,
			rng
		);

		add(ret.pixels(), std::as_const(tmp).pixels());
		++current_trunk_index;
	}
	return ret;
}

terraformer::grayscale_image
terraformer::ridge_tree_descriptor::generate_heightmap(heightmap_generator_context const& ctxt) const
{ return generate(ctxt, *this); }

void terraformer::ridge_tree_trunk_control_point_descriptor::bind(descriptor_editor_ref editor)
{
	editor.create_float_input(
		u8"x",
		x,
		descriptor_editor_ref::knob_descriptor{
			.textbox_placeholder_string = u8"0.00019329926",
			.visual_angle_range = std::nullopt
		}
	);

	editor.create_float_input(
		u8"y",
		y,
		descriptor_editor_ref::knob_descriptor{
			.textbox_placeholder_string = u8"0.00019329926",
			.visual_angle_range = std::nullopt
		}
	);

	editor.create_float_input(
		u8"Heading",
		heading,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::affine_value_map{-0.5f, 0.5f}},
			.textbox_placeholder_string = u8"-0.123456789",
			.visual_angle_range = closed_closed_interval<geosimd::turn_angle>{
				geosimd::turns{0.0},
				geosimd::turns{1.0}
			}
		}
	);

	editor.create_float_input(
		u8"Speed",
		speed,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{1.0f/128.0f, 2.0f, 2.0f}},
			.textbox_placeholder_string = u8"0.011031073",
			.visual_angle_range = std::nullopt
		}
	);
}

void terraformer::ridge_tree_trunk_descriptor::bind(descriptor_editor_ref editor)
{
	auto point_table = editor.create_table(
		descriptor_editor_ref::field_descriptor{
			.label = u8"Curve"
		},
		descriptor_editor_ref::table_descriptor{
			.orientation = descriptor_editor_ref::widget_orientation::deduce,
			.field_names{
				u8"x",
				u8"y",
				u8"Heading",
				u8"Speed",
			}
		}
	);

	{
		auto record = point_table.add_record(u8"Begin");
		curve.begin.bind(record);
		record.append_pending_widgets();
	}

	{
		auto record = point_table.add_record(u8"End");
		curve.end.bind(record);
		record.append_pending_widgets();
	}

	auto starting_point_branches_form = editor.create_form(
		descriptor_editor_ref::field_descriptor{
			.label = u8"Starting point branches"
		},
		descriptor_editor_ref::form_descriptor{
		}
	);
	starting_point_branches.bind(starting_point_branches_form);

	editor.create_float_input(
		u8"Ridge height/m",
		ridge_height,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{1.0f, 8192.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);
}

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

void terraformer::ridge_tree_branch_growth_descriptor::bind(descriptor_editor_ref editor)
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

	editor.create_float_input(
		u8"Rel. height",
		begin_height,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{0.25f, 4.0f, 2.0f}},
			.textbox_placeholder_string = u8"0.123456789",
			.visual_angle_range = std::nullopt
		}
	);
}

void terraformer::ridge_tree_height_profile_descriptor::bind(descriptor_editor_ref editor)
{
	editor.create_float_input(
		u8"Rel. half thickness",
		rel_half_thickness,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{0.25f, 4.0f, 2.0f}},
			.textbox_placeholder_string = u8"0.123456789",
			.visual_angle_range = std::nullopt
		}
	);
	editor.create_float_input(
		u8"Roll-off exponent",
		rolloff_exponent,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{0.25f, 4.0f, 2.0f}},
			.textbox_placeholder_string = u8"0.123456789",
			.visual_angle_range = std::nullopt
		}
	);
	editor.create_float_input(
		u8"Noise wavelength/m",
		noise_wavelength,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{128.0f, 65536.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);
	editor.create_float_input(
		u8"Noise LF roll-off",
		noise_lf_rolloff,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{1.0f, 8.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);
	editor.create_float_input(
		u8"Noise HF roll-off",
		noise_hf_rolloff,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{1.0f, 8.0f, 2.0f}},
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
}

void terraformer::ridge_tree_descriptor::bind(descriptor_editor_ref editor)
{
	editor.create_rng_seed_input(u8"Seed", rng_seed);

	auto trunk_editor = editor.create_form(
		descriptor_editor_ref::field_descriptor{
			.label = u8"Trunk"
		},
		descriptor_editor_ref::form_descriptor{
			.orientation = descriptor_editor_ref::widget_orientation::vertical
		}
	);

	trunk.bind(trunk_editor);

	{
		auto ep_branches_table = editor.create_table(
			descriptor_editor_ref::field_descriptor{
				.label = u8"Endpoint branches"
			},
			descriptor_editor_ref::table_descriptor{
				.orientation = descriptor_editor_ref::widget_orientation::horizontal,
				.field_names{
					u8"Branch count",
					u8"Spread angle"
				}
			}
		);

		size_t k = 0;
		for(auto& item : endpoint_branches)
		{
			auto record = ep_branches_table.add_record(
				k == 0? u8"Trunk" : reinterpret_cast<char8_t const*>(std::to_string(k).c_str())
			);
			item.bind(record);
			record.append_pending_widgets();
			++k;
		}
	}

	{
		auto branch_growth_table = editor.create_table(
			descriptor_editor_ref::field_descriptor{
				.label = u8"Branch growth"
			},
			descriptor_editor_ref::table_descriptor{
				.orientation = descriptor_editor_ref::widget_orientation::horizontal,
				.field_names{
					u8"E2E distance/m",
					u8"Rel. height"
				}
			}
		);

		size_t k = 0;
		for(auto& item : branch_growth_params)
		{
			auto record = branch_growth_table
				.add_record(reinterpret_cast<char8_t const*>(std::to_string(k + 1).c_str()));
			item.bind(record);
			record.append_pending_widgets();
			++k;
		}
	}

	{
		auto horz_displacement_table = editor.create_table(
			descriptor_editor_ref::field_descriptor{
				.label = u8"Horz displacements"
			},
			descriptor_editor_ref::table_descriptor{
				.orientation = descriptor_editor_ref::widget_orientation::horizontal,
				.field_names{
					u8"Amplitude/m",
					u8"Wavelength/m",
					u8"Damping"
				}
			}
		);
		size_t k = 0;
		for(auto& item : horz_displacements)
		{
			auto record = horz_displacement_table.add_record(
				k == 0? u8"Trunk" : reinterpret_cast<char8_t const*>(std::to_string(k).c_str())
			);
			item.bind(record);
			record.append_pending_widgets();
			++k;
		}
	}

	{
		auto elev_profile_table = editor.create_table(
			descriptor_editor_ref::field_descriptor{
				.label = u8"Ridge height profile"
			},
			descriptor_editor_ref::table_descriptor{
				.orientation = descriptor_editor_ref::widget_orientation::horizontal,
				.field_names{
					u8"Rel. half thickness",
					u8"Roll-off exponent",
					u8"Noise wavelength/m",
					u8"Noise LF roll-off",
					u8"Noise HF roll-off",
					u8"Noise amplitude/m"
				}
			}
		);
		size_t k = 0;
		for(auto& item : height_profile)
		{
			auto record = elev_profile_table.add_record(
				k == 0? u8"Trunk" : reinterpret_cast<char8_t const*>(std::to_string(k).c_str())
			);			item.bind(record);
			record.append_pending_widgets();
			++k;
		}
	}
}