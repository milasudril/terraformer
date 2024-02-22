//@	{"target": {"name":"./ridge_tree.o"}}

#include "./ridge_tree.hpp"
#include "./ridge_tree_branch.hpp"

#include "lib/curve_tools/rasterizer.hpp"

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

	if(std::size(curve_levels) == 0)
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
		multi_array<displaced_curve, displaced_curve::index_type> root;
		root.push_back(
			displace_xy(
				trunk_base_curve,
				terraformer::displacement_profile{
					.offsets = trunk_offsets,
					.sample_period = pixel_size
				}
			),
			displaced_curve::index_type{}
		);

		ret.push_back(
			ridge_tree_branch_collection{
				.level = 0,
				.curves = std::move(root),
				.parent = ridge_tree_branch_collection::no_parent,
				.parent_curve_index = array_index<displaced_curve>{0},
				.side = ridge_tree_branch_collection::side::left
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

		auto next_level_seeds = terraformer::collect_ridge_tree_branch_seeds(
			std::as_const(current_trunk.curves).get<0>()
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
					ridge_tree_branch_collection{
						.level = next_level_index,
						.curves = std::move(stem.left),
						.parent = current_trunk_index,
						.parent_curve_index = stem.parent_curve_index,
						.side = ridge_tree_branch_collection::side::left
					}
				);
			}

			if(!stem.right.empty())
			{
				ret.push_back(
					ridge_tree_branch_collection{
						.level = next_level_index,
						.curves = std::move(stem.right),
						.parent = current_trunk_index,
						.parent_curve_index = stem.parent_curve_index,
						.side = ridge_tree_branch_collection::side::right
					}
				);
			}
		}
		++current_trunk_index;
	}
}

void terraformer::ridge_tree::update_elevations(
	elevation,
	std::span<ridge_tree_branch_elevation_profile const> elevation_profiles,
	random_generator,
	float
)
{
	span<ridge_tree_branch_collection> branches{m_value};
	for(auto& current_collection : branches)
	{
		auto const level = current_collection.level;

		if(level >= std::size(elevation_profiles))
		{ return; }

		auto const parent = current_collection.parent;
		if(parent == ridge_tree_branch_collection::no_parent)
		{ continue; }

		auto const parent_curve_index = current_collection.parent_curve_index;
		auto const side = current_collection.side == ridge_tree_branch_collection::side::left? "left":"right";

		printf("level: %zu, parent: %zu, parent_curve_index: %zu, side: %s\n", level, parent.get(), parent_curve_index.get(), side);

		span<displaced_curve> parent_curves{branches[parent].curves.get<0>()};
		auto const my_curves = current_collection.curves.get<0>();
		auto const start_index = current_collection.curves.get<1>();

		for(auto k = current_collection.curves.first_element_index();
			k != std::size(current_collection.curves);
			++k
		)
		{
			printf("%zu starts at %zu %s %s\n",
				k.get(),
				start_index[k].get(),
				to_string(parent_curves[parent_curve_index].points()[start_index[k]]).c_str(),
				to_string(my_curves[k].points()[displaced_curve::index_type{}]).c_str()
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

		auto const peak_elevation = params.curve_levels[level].peak_elevation;
		auto const scaled_peak_diameter = 2.0f*params.curve_levels[level].peak_radius/pixel_size;

		for(auto const& curve: branch_collection.curves.get<0>())
		{
			draw(
				output,
				curve.points(),
				line_segment_draw_params{
					.value = peak_elevation,
					.blend_function = [](float old_val, float new_val, float strength){
						return std::max(old_val, new_val*strength);
					},
					.scale = pixel_size,
					.brush = [](float xi, float eta) {
						return std::max(1.0f - std::sqrt(xi*xi + eta*eta), 0.0f);
					},
					.brush_diameter = [scaled_peak_diameter](float, float){
						return scaled_peak_diameter;
					}
				}
			);
		}
	}
}