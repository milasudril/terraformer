//@	{"target": {"name":"./ridge_tree.o"}}

#include "./ridge_tree.hpp"
#include "./ridge_tree_branch.hpp"

#include "lib/curve_tools/length.hpp"
#include "lib/curve_tools/rasterizer.hpp"
#include "lib/math_utils/interp.hpp"
#include "lib/math_utils/cubic_spline.hpp"

namespace terraformer
{
	namespace
	{
		std::vector<location> make_point_array(location start_loc, direction dir, size_t count, float dx)
		{
			std::vector<location> ret(count);
			for(size_t k = 0; k != count; ++k)
			{
				ret[k] = start_loc + static_cast<float>(k)*dx*dir;
			}
			return ret;
		}
	}
}

terraformer::ridge_tree::ridge_tree(
	ridge_tree_description const& description,
	random_generator rng,
	float pixel_size
)
{
	auto& ret = m_value;
	std::span curve_levels{description.curve_levels};

	if(curve_levels.empty())
	{ return; }

	auto const trunk_pixel_count = static_cast<size_t>(curve_levels[0].growth_params.max_length/pixel_size);
	auto const trunk_offsets = generate(
		curve_levels[0].displacement_profile,
		rng,
		array_size<float>{trunk_pixel_count},
		pixel_size,
		1024.0f);

	auto const trunk_base_curve = terraformer::make_point_array
		(description.root_location, description.trunk_direction, trunk_pixel_count, pixel_size);

	{
		ridge_tree_branch_sequence root;
		root.push_back(
			displace_xy(
				trunk_base_curve,
				terraformer::displacement_profile{
					.offsets = trunk_offsets,
					.sample_period = pixel_size
				}
			),
			displaced_curve::index_type{},
			single_array<displaced_curve::index_type>{}
		);

		ret.push_back(
			ridge_tree_trunk{
				.level = 0,
				.branches = std::move(root),
				.parent = ridge_tree_trunk::no_parent,
				.parent_curve_index = array_index<displaced_curve>{0},
				.side = ridge_tree_trunk::side::left
			}
		);
	}

	auto current_trunk_index = ret.first_element_index();

	while(true)
	{
		if(current_trunk_index == std::size(ret))
		{ return; }

		auto& current_trunk = ret[current_trunk_index];
		auto const next_level_index = current_trunk.level  + 1;
		if(next_level_index == std::size(curve_levels))
		{
			++current_trunk_index;
			continue;
		}

		auto const next_level_seeds = terraformer::collect_ridge_tree_branch_seeds(
			std::as_const(current_trunk.branches).get<0>()
		);
		auto next_level = generate_branches(
			next_level_seeds,
			ret,
			pixel_size,
			curve_levels[next_level_index].displacement_profile,
			rng,
			curve_levels[next_level_index].growth_params
		);

		for(auto& stem: next_level)
		{
			if(!stem.left.empty())
			{
				ret.push_back(
					ridge_tree_trunk{
						.level = next_level_index,
						.branches = std::move(stem.left),
						.parent = current_trunk_index,
						.parent_curve_index = stem.parent_curve_index,
						.side = ridge_tree_trunk::side::left
					}
				);
			}

			if(!stem.right.empty())
			{
				ret.push_back(
					ridge_tree_trunk{
						.level = next_level_index,
						.branches = std::move(stem.right),
						.parent = current_trunk_index,
						.parent_curve_index = stem.parent_curve_index,
						.side = ridge_tree_trunk::side::right
					}
				);
			}
		}
		++current_trunk_index;
	}
}

void terraformer::ridge_tree::update_elevations(
	elevation initial_elevation,
	std::span<ridge_tree_branch_elevation_profile const> elevation_profiles,
	random_generator,
	float
)
{
	span<ridge_tree_trunk> branches{m_value};
	for(auto& current_collection : branches)
	{
		auto const level = current_collection.level;

		if(level >= std::size(elevation_profiles))
		{ return; }

		auto const my_curves = current_collection.branches.get<0>();
		auto const start_index = current_collection.branches.get<1>();
		auto const parent = current_collection.parent;
		if(parent == ridge_tree_trunk::no_parent)
		{
			for(auto& curve : my_curves)
			{
				replace_z_inplace(
					curve.points(),
					make_interpolator(
						cubic_spline_control_point{
							.y = initial_elevation,
							.ddx = std::tan(2.0f*std::numbers::pi_v<float>*elevation_profiles[level].starting_slope)
						},
						cubic_spline_control_point{
							.y = elevation_profiles[level].final_elevation,
							.ddx = std::tan(2.0f*std::numbers::pi_v<float>*elevation_profiles[level].final_slope)
						}
					)
				);
			}
			continue;
		}

		auto const parent_curves = branches[parent].branches.get<0>().decay();
		auto const parent_curve_index = current_collection.parent_curve_index;
		auto const parent_curve = parent_curves[parent_curve_index].points();

		for(auto k = current_collection.branches.first_element_index();
			k != std::size(current_collection.branches);
			++k
		)
		{
			auto const point_on_parent = parent_curve[start_index[k]];
			auto const z_0 = point_on_parent[2];
			replace_z_inplace(
				my_curves[k].points(),
				make_interpolator(
					cubic_spline_control_point{
						.y = z_0,
						.ddx = std::tan(2.0f*std::numbers::pi_v<float>*elevation_profiles[level].starting_slope)
					},
					cubic_spline_control_point{
						.y = elevation_profiles[level].final_elevation,
						.ddx = std::tan(2.0f*std::numbers::pi_v<float>*elevation_profiles[level].final_slope)
					}
				)
			);
		}
	}
}

void terraformer::render(
	ridge_tree const& tree,
	span_2d<float> output,
	ridge_tree_render_description const& params,
	float pixel_size
)
{
	for(auto const& branch_collection: tree)
	{
		auto const level = branch_collection.level;
		if(level >= std::size(params.curve_levels))
		{ continue; }

	//	auto const peak_elevation = params.curve_levels[level].peak_elevation;
		auto const peak_diameter = 2.0f/pixel_size;

		for(auto const& branch: branch_collection.branches.get<0>())
		{
			draw(
				output,
				branch.points(),
				line_segment_draw_params{
					.value = 1.0f,
					.blend_function = [](float old_val, float new_val, float strength){
						return std::max(old_val, new_val*strength);
					},
					.scale = pixel_size,
					.brush = [](float xi, float eta) {
						return std::max(1.0f - std::sqrt(xi*xi + eta*eta), 0.0f);
					},
					.brush_diameter = [peak_diameter](float, float, float z){
						return z*peak_diameter;
					}
				}
			);
		}
	}
}