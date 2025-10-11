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
#include <cfenv>

// TODO:
//
// To add:
//
// * Control Roll-off exponent for ridge elevation
// * Avoid branch early. Looks strange, especially at the root curve
// * Multi-threading where possible
//
// To verify:
//
// * Branch pruning
//
//
// Experiments:
//
// Strategy 1:
//
// * Try to add noise before tracing children. Set the noise amplitude as a factor, with an
//   amplitude of 0.25*input_elevation. Wave function with offset 1.
//
//
// Strategy 2:
//
// * Try to add extra elevation at vertices. Use at most 0.25*input_elevation. Actual value is
//   randomized. Wave function with offset 1.
// * Add noise with an amplitude of 0.125*input_elevation. Wave function with offset 1. If the added
//   does not work together with tracer, apply it after the heightmap has been used to generate
//   children.

namespace
{
#if 0
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
	void add_circle(
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

				if( r <= r_0)
				{ output(x, y) = output(x,y) + shape(1.0f - r); }
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
#endif

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

#if 0
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
#endif
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
	auto const transverse_rolloff_exponent = elev_profile.transverse_rolloff_exponent;
	auto const longitudinal_rolloff_exponent = elev_profile.longitudinal_rolloff_exponent;
	auto const begin_height_is_relative = elev_profile.begin_height_is_relative;
	auto const begin_height_in = elev_profile.begin_height;
	auto const end_height_in = elev_profile.end_height;
	auto const ridge_radius = elev_profile.relative_half_thickness/pixel_size;

	for(auto k : elems)
	{
		auto const& curve = curves[k];
		if(curve.points().empty())
		{ continue; }

		auto const start_loc = (curve.points().front() - location{})/pixel_size;
		auto const begin_height =
			begin_height_in*(
				begin_height_is_relative?
					interp(pixels_in, start_loc[0], start_loc[1], clamp_at_boundary{}) :
					1.0f
			);
		auto const end_height = end_height_in*begin_height;

		float curve_length = 0.0f;
		visit_pixels(
			curve.points(),
			pixel_size,
			[
				loc_prev = location{} + (curve.points().front() - location{})/pixel_size,
				&curve_length
			](location loc, auto&&...) mutable {
				curve_length += distance(loc, loc_prev);
				loc_prev = loc;
			}
		);

		visit_pixels(
			curve.points(),
			pixel_size,
			[
				pixels,
				ridge_radius,
				transverse_rolloff_exponent,
				longitudinal_rolloff_exponent,
				begin_height,
				end_height,
				loc_prev = location{} + (curve.points().front() - location{})/pixel_size,
				integrated_length = 0.0f,
				curve_length
			](location loc, direction tangent, direction normal) mutable {
				auto const curve_param = integrated_length/curve_length;
				auto const current_height =
					std::lerp(
						end_height,
						begin_height,
						std::pow(1.0f - curve_param, longitudinal_rolloff_exponent)
					);

				max_circle(
					pixels,
					loc,
					tangent,
					normal,
					ridge_radius*current_height,
					[
						transverse_rolloff_exponent,
						current_height
					](float r){
						return current_height*std::pow(r, transverse_rolloff_exponent);
					},
					2.0f
				);

				integrated_length += distance(loc, loc_prev);
				loc_prev = loc;
			}
		);
	}
}

namespace
{
#if 0
	void pick_max(terraformer::span_2d<float> output, terraformer::span_2d<float const> input)
	{
		for(uint32_t y = 0; y != output.height(); ++y)
		{
			for(uint32_t x = 0; x != output.width(); ++x)
			{
				output(x, y) = std::max(output(x, y), input(x, y));
			}
		}
	}
#endif
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

	fill_curve(
		ret,
		span_2d<float const>{},
		trunks.back(),
		ridge_tree_ridge_height_profile{
			.begin_height = params.trunk.ridge_height,
			.begin_height_is_relative = false,
			.end_height  = 1.0f,
			.relative_half_thickness = 1.25f,
			.transverse_rolloff_exponent = 1.25f,
			.longitudinal_rolloff_exponent = 1.0f
		},
		global_pixel_size
	);
#if 0
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

		auto const next_level_seeds = collect_ridge_tree_branch_seeds(
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

		auto k = next_level_seeds.element_indices().front();
		for(auto& index_array : current_trunk.branches.get<2>())
		{
			index_array = collect_branch_indices(next_level_seeds[k]);
			++k;
		}

		auto const& horz_displacement = displacement_profiles[next_level_index];
		auto const& growth_params = branch_growth_params[next_level_index - 1];
		auto const& elev_profile = params.elevation_profile[next_level_index];
		//auto const pixel_size = get_min_pixel_size(horz_displacement);

		feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW );
		auto next_level = generate_branches(
			next_level_seeds,
			ret.pixels(),
			global_pixel_size,
			ridge_tree_branch_displacement_description{
				.amplitude = horz_displacement.amplitude,
				.wavelength = horz_displacement.wavelength,
				.damping = horz_displacement.damping
			},
			rng,
			ridge_tree_branch_growth_description{
				.max_length = growth_params.e2e_distance,
				.min_neighbour_distance = 1.25f*(
					horz_displacement.amplitude + 0.5f*elev_profile.ridge_half_thickness
				)
			}
		);

		fedisableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW );
		for(auto& stem: next_level)
		{
			grayscale_image tmp_left{w_img, h_img};
			grayscale_image tmp_right{w_img, h_img};

			if(!stem.left.empty())
			{
				trunks.push_back(
					ridge_tree_trunk{
						.level = next_level_index,
						.branches = std::move(stem.left),
						.parent = current_trunk_index,
						.parent_curve_index = stem.parent_curve_index,
						.side = ridge_tree_trunk::side::left,
						.elevation_data = ridge_tree_branch_elevation_data{}
					}
				);

				fill_curve(
					tmp_left,
					ret.pixels(),
					trunks.back(),
					params.elevation_profile[next_level_index],
					global_pixel_size,
					ridge_tree_ridge_thickness_modulation{
						.begin_val = 1.0f,
						.end_val = 0.5f,
						.rel_height = true
					}
				);
			}

			if(!stem.right.empty())
			{
				trunks.push_back(
					ridge_tree_trunk{
						.level = next_level_index,
						.branches = std::move(stem.right),
						.parent = current_trunk_index,
						.parent_curve_index = stem.parent_curve_index,
						.side = ridge_tree_trunk::side::right,
						.elevation_data = ridge_tree_branch_elevation_data{}
					}
				);

				fill_curve(
					tmp_right,
					ret.pixels(),
					trunks.back(),
					params.elevation_profile[next_level_index],
					global_pixel_size,
					ridge_tree_ridge_thickness_modulation{
						.begin_val = 1.0f,
						.end_val = 0.5f,
						.rel_height = true
					}
				);
			}

			pick_max(ret.pixels(), std::as_const(tmp_left).pixels());
			pick_max(ret.pixels(), std::as_const(tmp_right).pixels());
		}

		++current_trunk_index;
	}
#endif
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
		u8"Rel. end height",
		end_height,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{0.25f, 4.0f, 2.0f}},
			.textbox_placeholder_string = u8"0.123456789",
			.visual_angle_range = std::nullopt
		}

	);

	editor.create_float_input(
		u8"Longit. roll-off exp.",
		longitudinal_rolloff_exponent,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{0.25f, 4.0f, 2.0f}},
			.textbox_placeholder_string = u8"0.123456789",
			.visual_angle_range = std::nullopt
		}
	);
}

void terraformer::ridge_tree_elevation_profile_descriptor::bind(descriptor_editor_ref editor)
{
	editor.create_float_input(
		u8"Ridge half-thickness",
		ridge_half_thickness,
		descriptor_editor_ref::knob_descriptor{
			.value_map = type_erased_value_map{value_maps::log_value_map{0.25f, 4.0f, 2.0f}},
			.textbox_placeholder_string = u8"0.123456789",
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
					u8"Rel. end height",
					u8"Longit. roll-off exp."
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
				.label = u8"Elevation profile"
			},
			descriptor_editor_ref::table_descriptor{
				.orientation = descriptor_editor_ref::widget_orientation::horizontal,
				.field_names{
					u8"Ridge half-thickness",
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