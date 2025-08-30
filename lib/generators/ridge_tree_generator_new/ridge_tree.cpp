//@	{"target": {"name":"./ridge_tree.o"}}

#include "./ridge_tree.hpp"
#include "./ridge_tree_branch.hpp"

#include "lib/common/utils.hpp"
#include "lib/curve_tools/length.hpp"
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
	ridge_tree_xy_description const& description,
	random_generator& rng
)
{
	auto& ret = m_value;
	std::span branch_growth_params{description.branch_growth_params};
	std::span displacement_profiles{description.displacement_profiles};

	if(branch_growth_params.empty() || displacement_profiles.empty())
	{ return; }

	auto const trunk_pixel_size = get_min_pixel_size(displacement_profiles[0]);
	auto const trunk_pixel_count = static_cast<size_t>(description.trunk_growth_params.max_length/trunk_pixel_size);

	auto const trunk_offsets = generate(
		displacement_profiles[0],
		rng,
		array_size<float>{trunk_pixel_count},
		trunk_pixel_size,
		1024.0f
	);

	auto const trunk_base_curve = terraformer::make_point_array
		(description.root_location, description.trunk_direction, trunk_pixel_count, trunk_pixel_size);
	{
		auto curve = displace_xy(
			trunk_base_curve,
			terraformer::displacement_profile{
				.offsets = trunk_offsets,
				.sample_period = trunk_pixel_size
			}
		);

		auto integrated_curve_length = curve_running_length_xy(std::as_const(curve).points());

		ridge_tree_branch_sequence root;
		root.push_back(
			std::move(curve),
			displaced_curve::index_type{},
			single_array<displaced_curve::index_type>{},
			std::move(integrated_curve_length)
		);

		ret.push_back(
			ridge_tree_trunk{
				.level = 0,
				.branches = std::move(root),
				.parent = ridge_tree_trunk::no_parent,
				.parent_curve_index = array_index<displaced_curve>{0},
				.side = ridge_tree_trunk::side::left,
				.elevation_data = ridge_tree_branch_elevation_data{}
			}
		);
	}

	// TODO: Can we use a range-based loop now?
	auto current_trunk_index = ret.element_indices().front();
	while(true)
	{
		if(current_trunk_index == std::size(ret))
		{ return; }

		auto& current_trunk = ret[current_trunk_index];
		auto const next_level_index = current_trunk.level + 1;
		if(next_level_index == std::size(branch_growth_params) + 1)
		{
			++current_trunk_index;
			continue;
		}

		auto const next_level_seeds = terraformer::collect_ridge_tree_branch_seeds(
			std::as_const(current_trunk.branches).get<0>()
		);

		auto k = next_level_seeds.element_indices().front();
		for(auto& index_array : current_trunk.branches.get<2>())
		{
			index_array = collect_branch_indices(next_level_seeds[k]);
			++k;
		}

		auto const pixel_size = get_min_pixel_size(displacement_profiles[next_level_index]);
		auto next_level = generate_branches(
			next_level_seeds,
			ret,
			pixel_size,
			displacement_profiles[next_level_index],
			rng,
			branch_growth_params[next_level_index - 1]
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
						.side = ridge_tree_trunk::side::left,
						.elevation_data = ridge_tree_branch_elevation_data{}
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
						.side = ridge_tree_trunk::side::right,
						.elevation_data = ridge_tree_branch_elevation_data{}
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
	random_generator& rng
)
{
	span<ridge_tree_trunk> branches{m_value};
	for(auto& current_trunk : branches)
	{
		current_trunk.elevation_data.clear();
		auto const level = current_trunk.level;

		if(level >= std::size(elevation_profiles))
		{ return; }

		auto const my_curves = current_trunk.branches.get<0>();
		auto const curve_lengths = std::as_const(current_trunk.branches).get<3>();
		auto const parent = current_trunk.parent;
		if(parent == ridge_tree_trunk::no_parent)
		{
			for(auto k : current_trunk.branches.element_indices())
			{
				auto const polynomial = create_polynomial(
					curve_lengths[k].back(),
					initial_elevation,
					elevation_profiles[level].base_elevation,
					rng
				);

				auto const elevation_profile = generate_elevation_profile(
					curve_lengths[k],
					polynomial
				);

				replace_z_inplace(my_curves[k].points(), elevation_profile);
				current_trunk.elevation_data.push_back(polynomial);
			}
			continue;
		}

		auto const start_index = current_trunk.branches.get<1>();
		auto const parent_curves = branches[parent].branches.get<0>().decay();
		auto const parent_curve_index = current_trunk.parent_curve_index;
		auto const parent_curve = parent_curves[parent_curve_index].points();

		for(auto k : current_trunk.branches.element_indices())
		{
			auto const point_on_parent = parent_curve[start_index[k]];
			elevation const z_0{point_on_parent[2]};
			auto const elev_function = create_polynomial(
				curve_lengths[k].back(),
				z_0,
				elevation_profiles[level].base_elevation,
				rng
			);

			auto const elevation_profile = generate_elevation_profile(curve_lengths[k], elev_function);
			replace_z_inplace(my_curves[k].points(), elevation_profile);
			current_trunk.elevation_data.push_back(elev_function);
		}
	}

	for(auto& current_trunk : branches)
	{
		auto const level = current_trunk.level;
		if(level >= std::size(elevation_profiles))
		{ return; }


		auto const my_curves = current_trunk.branches.get<0>();
		auto const branches_at = std::as_const(current_trunk.branches).get<2>();
		auto const curve_lengths = std::as_const(current_trunk.branches).get<3>();

		for(auto k : current_trunk.branches.element_indices())
		{
			auto const elevation_profile = generate_elevation_profile(
				curve_lengths[k],
				branches_at[k],
				current_trunk.elevation_data[array_index<polynomial<3>>{k.get()}],
				elevation_profiles[level].modulation,
				rng
			);

			replace_z_inplace(my_curves[k].points(), elevation_profile);
		}
	}
}