//@	{"target": {"name": "./ridge_tree_generator.o"}}

#include "./ridge_tree_generator.hpp"
#include "./ridge_tree.hpp"

#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/common/value_map.hpp"
#include "lib/generators/domain/domain_size.hpp"
#include "lib/generators/heightmap/heightmap_generator_context.hpp"
#include "lib/generators/ridge_tree_generator_new/ridge_curve.hpp"
#include "lib/generators/ridge_tree_generator_new/ridge_tree_branch.hpp"
#include "lib/generators/ridge_tree_generator_new/ridge_tree_branch_seed_sequence.hpp"
#include "lib/math_utils/butter_bp_2d.hpp"
#include "lib/math_utils/butter_lp_2d.hpp"
#include "lib/math_utils/cubic_spline.hpp"
#include "lib/math_utils/interp.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/value_maps/qurt_value_map.hpp"
#include "lib/value_maps/log_value_map.hpp"
#include "lib/curve_tools/rasterizer.hpp"


#include "lib/curve_tools/dump.hpp"
#include "lib/common/cfile_owner.hpp"

#include <algorithm>
#include <cassert>
#include <geosimd/unit_vector.hpp>
#include <numbers>
#include <random>

namespace
{
	auto make_cubic_spline_control_point(
		terraformer::domain_size_descriptor dom_size,
		terraformer::ridge_tree_trunk_control_point_descriptor const& params
	)
	{
		auto const theta = 2.0f*std::numbers::pi_v<float>*params.heading;
		auto const r = terraformer::displacement{params.x, params.y, 0.0f}
			.apply(terraformer::scaling{dom_size.width, dom_size.height, 1.0f});
		auto const dom_norm = std::sqrt(dom_size.width*dom_size.height);
		return terraformer::cubic_spline_control_point<terraformer::location, terraformer::displacement>{
			.y = terraformer::location{} + r,
			.ddx = dom_norm*params.speed*terraformer::displacement{std::sin(theta), -std::cos(theta), 0.0f}
		};
	}

	terraformer::ridge_tree_trunk_curve_descriptor
	make_ridge_tree_trunk_description(
		terraformer::domain_size_descriptor dom_size,
		terraformer::ridge_tree_trunk_descriptor const& params
	)
	{
		return terraformer::ridge_tree_trunk_curve_descriptor{
			.begin = make_cubic_spline_control_point(dom_size, params.begin),
			.end = make_cubic_spline_control_point(dom_size, params.end)
		};
	}

	terraformer::ridge_tree_xy_description
	collect_ridge_tree_xy_params(
		terraformer::domain_size_descriptor dom_size,
		terraformer::ridge_tree_descriptor const& params
	)
	{
		std::vector<terraformer::ridge_tree_branch_growth_description> branch_growth_params;
		for(size_t k = 0; k != std::size(params.branch_growth_params); ++k)
		{
			auto const& item =  params.branch_growth_params[k];
			branch_growth_params.push_back(
				terraformer::ridge_tree_branch_growth_description{
					.max_length = item.e2e_distance,
					.min_neighbour_distance = 2.0f*params.horz_displacements[k + 1].amplitude
				}
			);
		}

		std::vector<terraformer::ridge_tree_branch_displacement_description> displacement_profiles;
		for(size_t k = 0; k != std::size(params.horz_displacements); ++k)
		{
			auto const& item = params.horz_displacements[k];
			displacement_profiles.push_back(
				terraformer::ridge_tree_branch_displacement_description{
					.amplitude = item.amplitude,
					.wavelength = item.wavelength,
					.damping = item.damping
				}
			);
		}

		return terraformer::ridge_tree_xy_description{
			.trunk = make_ridge_tree_trunk_description(dom_size, params.trunk),
			.branch_growth_params = std::move(branch_growth_params),
			.displacement_profiles = std::move(displacement_profiles),
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
	void  max_circle(
		terraformer::span_2d<float> output,
		terraformer::location loc,
		terraformer::direction tangent,
		terraformer::direction normal,
		float r, Shape&& shape
	)
	{
		auto const x_0 = loc[0];
		auto const y_0 = loc[1];
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
				auto const v = (
					  (terraformer::location{static_cast<float>(x), static_cast<float>(y), 0.0f} - loc)
					+ terraformer::displacement{0.5f, 0.5f, 0.0f}
					)/r;

				auto const xi = inner_product(v, tangent);
				auto const eta = inner_product(v, normal);

				auto const r = std::abs(xi) + std::abs(eta);
				if( r <= 1.0f)
				{ output(x, y) = std::max(output(x,y), shape(1.0f - r)); }
			}
		}
	}

	auto render_branches_at_current_level(
		terraformer::heightmap_generator_context const& ctxt,
		float min_pixel_size,
		terraformer::ridge_tree_elevation_profile_descriptor const& elevation_profile,
		terraformer::ridge_tree_trunk const* i,
		terraformer::ridge_tree_trunk const* i_end,
		terraformer::random_generator& rng,
		terraformer::span_2d<float> output_image
	)
	{
		auto const dom_size = ctxt.domain_size;
		auto const level = i->level;
		auto const pixel_size = std::min(min_pixel_size, get_min_pixel_size(elevation_profile));
		auto const w_img_ridge = 2u*std::max(static_cast<uint32_t>(dom_size.width/(2.0f*pixel_size) + 0.5f), 1u);
		auto const h_img_ridge = 2u*std::max(static_cast<uint32_t>(dom_size.height/(2.0f*pixel_size) + 0.5f), 1u);

		terraformer::grayscale_image ridge{w_img_ridge, h_img_ridge};
		auto const ridge_radius = elevation_profile.ridge_half_thickness/pixel_size;
		auto const shape_exponent = elevation_profile.ridge_rolloff_exponent;
		printf("Rendering level %zu\n", level);
		while(i != i_end)
		{
			if(i->level != level)
			{ break; }

			auto const& branches = i->branches;
			auto const curves = branches.get<0>();
		//	auto const curve_lengths = branches.get<3>();
			for(auto k : branches.element_indices())
			{
				auto const& curve = curves[k];
				if(curve.points().empty())
				{ continue; }

				visit_pixels(curve.points(), pixel_size, [
					ridge = ridge.pixels(),
					&rng,
					ridge_radius,
					shape_exponent,
					ridge_elevation = 1.0f
				](terraformer::location loc, terraformer::direction tangent, terraformer::direction normal) {
					max_circle(ridge, loc, tangent, normal, ridge_radius, [
						shape_exponent,
						ridge_elevation
					](float r){
						return ridge_elevation*std::pow(r, shape_exponent);
					});
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
						ridge_elevation = elevation_profile.ridge_elevation
					](auto val) {
						return ridge_elevation * (val - min)/(max - min);
					}
				);
			}
		}

		terraformer::grayscale_image noise{w_img_ridge, h_img_ridge};
		{
			for(uint32_t y = 0; y != h_img_ridge; ++y)
			{
				for(uint32_t x = 0; x != w_img_ridge; ++x)
				{ noise(x,y) = std::uniform_real_distribution{0.0f, 1.0f}(rng); }
			}

			noise = terraformer::apply(
				terraformer::butter_bp_2d_descriptor{
					.f_x = 2.0f*dom_size.width/elevation_profile.noise_wavelength,
					.f_y = 2.0f*dom_size.height/elevation_profile.noise_wavelength,
					.lf_rolloff = elevation_profile.noise_lf_rolloff,
					.hf_rolloff = elevation_profile.noise_hf_rolloff,
					.y_direction = 0.0f
				},
				noise.pixels(),
				ctxt.comp_ctxt
			);

			auto const minmax = std::minmax_element(
				noise.pixels().data(),
				noise.pixels().data() + w_img_ridge*h_img_ridge
			);

			if(*minmax.first < *minmax.second)
			{
				std::transform(
					noise.pixels().data(),
					noise.pixels().data() + w_img_ridge*h_img_ridge,
					noise.pixels().data(),
					[
						min = *minmax.first,
						max = *minmax.second,
						noise_amplitude = elevation_profile.noise_amplitude
					](auto val) {
						return 2.0f*noise_amplitude*((val - min)/(max - min) - 0.5f);
					}
				);
			}
		}

		terraformer::grayscale_image tmp{w_img_ridge, h_img_ridge};
		for(uint32_t y = 0; y != h_img_ridge; ++y)
		{
			for(uint32_t x = 0; x != w_img_ridge; ++x)
			{
				tmp(x, y) = std::max(
					ridge(x, y)*(1.0f + noise(x, y)/elevation_profile.ridge_elevation),
					0.0f
				);
			}
		}

		terraformer::add_resampled(std::as_const(tmp).pixels(), output_image, 1.0f);
		return i;
	}
}

float terraformer::get_min_pixel_size(terraformer::ridge_tree_descriptor const& params)
{
	auto const min_layout = std::ranges::min_element(
		params.horz_displacements,
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
terraformer::generate(terraformer::heightmap_generator_context const& ctxt, ridge_tree_descriptor const& params)
{
	auto const rng_seed = std::bit_cast<terraformer::rng_seed_type>(params.rng_seed);
	terraformer::random_generator rng{rng_seed};

	auto const dom_size = ctxt.domain_size;
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
		i = render_branches_at_current_level(
			ctxt,
			0.5f*get_min_pixel_size(params.horz_displacements[i->level]),
			params.elevation_profile[i->level],
			i,
			std::end(ridge_tree),
			rng,
			ret.pixels()
		);
	}

	std::transform(
		ret.pixels().data(),
		ret.pixels().data() + w_img*h_img,
		ret.pixels().data(),
		[](auto val) {
			return std::max(0.0f, val);
		}
	);

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
		u8"Ridge half-thickness/m",
		ridge_half_thickness,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{128.0f, 65536.0f, 2.0f}},
			.textbox_placeholder_string = u8"9999.9999",
			.visual_angle_range = std::nullopt
		}
	);
	editor.create_float_input(
		u8"Ridge roll-off exponent",
		ridge_rolloff_exponent,
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

	{
		auto trunk_table = editor.create_table(
			descriptor_editor_ref::field_descriptor{
				.label = u8"Trunk"
			},
			descriptor_editor_ref::table_descriptor{
				.orientation = descriptor_editor_ref::widget_orientation::vertical,
				.field_names{
					u8"x",
					u8"y",
					u8"Heading",
					u8"Speed",
				}
			}
		);

		{
			auto record = trunk_table.add_record(u8"Begin");
			trunk.begin.bind(record);
			record.append_pending_widgets();
		}

		{
			auto record = trunk_table.add_record(u8"End");
			trunk.end.bind(record);
			record.append_pending_widgets();
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
					u8"E2E distance/m"
				}
			}
		);

		size_t k = 0;
		for(auto& item : branch_growth_params)
		{
			auto record = branch_growth_table.add_record(reinterpret_cast<char8_t const*>(std::to_string(k + 1).c_str()));
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
				.label = u8"Elevation profile"
			},
			descriptor_editor_ref::table_descriptor{
				.orientation = descriptor_editor_ref::widget_orientation::horizontal,
				.field_names{
					u8"Ridge elevation/m",
					u8"Ridge half-thickness/m",
					u8"Ridge roll-off exponent",
					u8"Noise wavelength/m",
					u8"Noise LF roll-off",
					u8"Noise HF roll-off",
					u8"Noise amplitude/m"
				}
			}
		);
		size_t k = 0;
		for(auto& item : elevation_profile)
		{
			auto record = elev_profile_table.add_record(
				k == 0? u8"Trunk" : reinterpret_cast<char8_t const*>(std::to_string(k).c_str())
			);			item.bind(record);
			record.append_pending_widgets();
			++k;
		}
	}
}