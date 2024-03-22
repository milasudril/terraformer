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
	ridge_tree_xy_description const& description,
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
		auto curve = displace_xy(
			trunk_base_curve,
			terraformer::displacement_profile{
				.offsets = trunk_offsets,
				.sample_period = pixel_size
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

		auto k = next_level_seeds.first_element_index();
		for(auto& index_array : current_trunk.branches.get<2>())
		{
			index_array = collect_branch_indices(next_level_seeds[k]);
			++k;
		}

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
	random_generator rng
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
			for(auto k = current_trunk.branches.first_element_index();
				k != std::size(current_trunk.branches);
				++k
			)
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

		for(auto k = current_trunk.branches.first_element_index();
			k != std::size(current_trunk.branches);
			++k
		)
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

		for(auto k = current_trunk.branches.first_element_index();
			k != std::size(current_trunk.branches);
			++k
		)
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

namespace
{
	class ridge_tree_brush
	{
	public:
		explicit ridge_tree_brush(float peak_radius, terraformer::span<terraformer::location const> locations, std::reference_wrapper<size_t> pixel_count):
			m_intensity_profile{},
			m_peak_radius{peak_radius},
			m_locations{locations},
			m_tangent{terraformer::direction{terraformer::displacement{1.0f, 0.0f, 0.0f}}},
			m_normal{terraformer::direction{terraformer::displacement{0.0f, 1.0f, 0.0f}}},
			m_pixel_count{pixel_count}
		{}

		void begin_pixel(float, float, float z, terraformer::array_index<terraformer::location> starting_at)
		{
			++m_pixel_count.get();

			if(starting_at < std::size(m_locations) - terraformer::array_size<terraformer::location>{1})
			{
				auto t = m_locations[starting_at + 1] - m_locations[starting_at - 1];
				t -= terraformer::displacement{0.0f, 0.0f, t[2]};
				m_tangent = terraformer::direction{t};
				// TODO: This operation should be optimized in geosimd
				m_normal = terraformer::direction{terraformer::displacement{m_tangent[1], -m_tangent[0], 0.0f}};
			}

			m_current_radius = z*m_peak_radius;
			m_intensity_profile = make_polynomial(
				terraformer::cubic_spline_control_point{
					.y = 1.0f,
					.ddx = -1.0f
				},
				terraformer::cubic_spline_control_point{
					.y = 0.0f,
					.ddx = 0.0f
				}
			);
		}

		auto get_radius() const
		{ return m_current_radius; }

		void get_pixel_value(float& old_val, float new_val, float xi, float eta) const
		{
			terraformer::displacement const v{xi, eta, 0.0f};
			auto const v_tangent = 3.0f*inner_product(v, m_tangent);
			auto const v_normal = inner_product(v, m_normal);
			auto const r = std::min(std::sqrt(v_tangent*v_tangent + v_normal*v_normal), 1.0f);
			auto const z = new_val*std::max(m_intensity_profile(r), 0.0f);
			old_val = std::max(old_val, z);
		}

	private:
		terraformer::polynomial<3> m_intensity_profile;
		float m_peak_radius;
		float m_current_radius;
		terraformer::span<terraformer::location const> m_locations;
		terraformer::direction m_tangent;
		terraformer::direction m_normal;

		std::reference_wrapper<size_t> m_pixel_count;
	};
}

#if 0
void terraformer::render(
	ridge_tree const& tree,
	span_2d<float> output,
	ridge_tree_render_description const& params,
	float pixel_size
)
{
	auto paint_mask = create_with_same_size<uint8_t>(output);

	for(auto const& branch_collection: tree)
	{
		auto const level = branch_collection.level;
		if(level >= std::size(params.curve_levels))
		{	continue; }

		auto const peak_radius = params.curve_levels[level].peak_radius.min;

		for(auto const& branch: branch_collection.branches.get<0>())
		{
			size_t pixel_count = 0;

			draw(
				output,
				branch.points(),
				line_segment_draw_params{
					.value = 1.0f,
					.scale = pixel_size
				},
				ridge_tree_brush{
					peak_radius/pixel_size,
					branch.points(),
					std::ref(pixel_count)
				},
				paint_mask.pixels()
			);
			printf("%zu\n", pixel_count);
			fflush(stdout);
		}
	}
}
#endif

void terraformer::render(
	ridge_tree const& tree,
	span_2d<float> output,
	ridge_tree_render_description const&,
	float pixel_size
)
{
	auto const branches = static_cast<span<ridge_tree_trunk const>>(tree);
	for(uint32_t k = 0; k != output.height(); ++k)
	{
		for(uint32_t l = 0; l != output.width(); ++l)
		{
			location const loc_scaled{static_cast<float>(l)*pixel_size, static_cast<float>(k)*pixel_size, 0.0f};
			auto const distance_result = closest_point_xy(branches, loc_scaled);
			if(distance_result.level == static_cast<size_t>(-1))
			{ abort(); }
#if 0
			printf("%s vs %s\n",
				to_string(distance_result.distance_result.loc).c_str(),
				to_string(loc_scaled).c_str()
			);
#endif
			auto const d_max = distance_result.distance_result.loc[2];
			output(l, k) = std::max(d_max - distance_result.distance_result.distance, 0.0f);
		}
	}
}