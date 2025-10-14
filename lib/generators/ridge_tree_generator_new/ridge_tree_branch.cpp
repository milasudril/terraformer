//@	{"target": {"name":"./ridge_tree_branch.o"}}

#include "./ridge_tree_branch.hpp"

#include "lib/curve_tools/length.hpp"
#include "lib/math_utils/first_order_hp_filter.hpp"
#include "lib/math_utils/second_order_lp_filter.hpp"

#include <fenv.h>

terraformer::polynomial<float, 3> terraformer::create_polynomial(
	float curve_length,
	elevation z_0,
	ridge_tree_ridge_elevation_profile_description const& elevation_profile,
	random_generator& rng
)
{
	constexpr auto two_pi = 2.0f*std::numbers::pi_v<float>;
	return make_polynomial(
		cubic_spline_control_point{
			.y = z_0,
			.ddx = -curve_length*std::tan(two_pi*pick(elevation_profile.starting_slope, rng))
		},
		cubic_spline_control_point{
			.y = elevation_profile.final_elevation,
			.ddx = -curve_length*std::tan(two_pi*pick(elevation_profile.final_slope, rng))
		}
	);
}

terraformer::single_array<float> terraformer::generate_elevation_profile(
	span<float const, array_index<float>, array_size<float>> integrated_curve_length,
	polynomial<float, 3> const& ridge_polynomial
)
{
	if(integrated_curve_length.empty())
	{ return terraformer::single_array<float>{}; }

	auto const L = integrated_curve_length.back();

	single_array ret{std::size(integrated_curve_length)};
	for(auto k : ret.element_indices())
	{ ret[k] = std::max(ridge_polynomial(integrated_curve_length[k]/L), 0.0f); }

	return ret;
}

namespace
{
	auto wrap_derivative(
		float target_derivative,
		float modulation_func_val,
		terraformer::polynomial<float, 3> const& f,
		float x,
		float x_scale_factor,
		float mod_depth
	)
	{
		auto const x_scaled = x/x_scale_factor;
		if(f(x_scaled) <= 0.0f || mod_depth <= 0.0f)
		{ return 0.0f; }

		return (
			 target_derivative
			- (1.0f + mod_depth*modulation_func_val)*f.derivative(x_scaled)/x_scale_factor
		)/(f(x_scaled)*mod_depth);
	}
}

terraformer::single_array<float> terraformer::generate_elevation_profile(
	span<float const> integrated_curve_length,
	span<displaced_curve::index_type const> branch_points,
	polynomial<float, 3> const& initial_elevation,
	ridge_tree_elevation_modulation_description const& elevation_profile,
	random_generator& rng
)
{
		// TODO: first element should probably use the post-modulated value from the parent as begin_elevation

	if(integrated_curve_length.empty())
	{ return terraformer::single_array<float>{}; }

	constexpr auto two_pi = 2.0f*std::numbers::pi_v<float>;
	auto const L = integrated_curve_length.back();
	auto const mod_depth = elevation_profile.mod_depth;
	auto const peak_noise_mix = elevation_profile.peak_noise_mix;
	auto const peak_gain = (1.0f - peak_noise_mix)*mod_depth;

	single_array noise_array{std::size(integrated_curve_length)};
	{
		std::uniform_real_distribution U{-1.0f, 1.0f};
		auto noise_gen = [
			hp = first_order_hp_filter{
				first_order_hp_filter_description{
					.cutoff_freq = two_pi/elevation_profile.elevation_noise.wavelength,
					.initial_value = 0.0f,
					.initial_input = 0.0f
				}
			},
			lp = second_order_lp_filter{
				second_order_lp_filter_description{
					.damping = elevation_profile.elevation_noise.damping,
					.cutoff_freq = two_pi/elevation_profile.elevation_noise.wavelength,
					.initial_value = 0.0f,
					.initial_derivative = 0.0f,
					.initial_input = 0.0f
				}
			}
		](float x, float dt) mutable {
			return lp(hp(x, dt), dt);
		};
		noise_array.front() = 0.0f;
		auto min_val = 2.0f;
		auto max_val = - min_val;
		// TODO: C++23 adjacent_view
		for(auto k : noise_array.element_indices(1))
		{
			auto const dt = integrated_curve_length[k] - integrated_curve_length[k - 1];
			auto const val = noise_gen(U(rng), dt);
			min_val = std::min(min_val, val);
			max_val = std::max(max_val, val);
			noise_array[k] = val;
		}

		auto const offset_in = 0.5f*(max_val + min_val);
		auto const amp_in = 0.5f*(max_val - min_val);

		for(auto& item : noise_array)
		{ item = (item - offset_in)/amp_in; }
	}

	single_array ret{std::size(integrated_curve_length)};
	{
		auto begin_elevation = 0.0f;
		auto begin_index = integrated_curve_length.element_indices().front();
		std::uniform_real_distribution peak_elevation_distribution{0.0f, 1.0f};
		for(auto branch_point_index : branch_points)
		{
			array_index<float> const end_index{branch_point_index.get()};
			auto const dl = integrated_curve_length[end_index] - integrated_curve_length[begin_index];
			auto const end_elevation = peak_elevation_distribution(rng);
			auto const col_elvation = -peak_elevation_distribution(rng);

			auto const begin_ddx = wrap_derivative(
				std::tan(two_pi*pick(elevation_profile.per_peak_modulation.slope, rng)),
				begin_elevation,
				initial_elevation,
				integrated_curve_length[begin_index],
				L,
				peak_gain
			);

			auto const end_ddx = wrap_derivative(
				std::tan(two_pi*pick(elevation_profile.per_peak_modulation.slope, rng)),
				begin_elevation,
				initial_elevation,
				integrated_curve_length[begin_index],
				L,
				peak_gain
			);

			auto const p_peak_begin = make_polynomial(
				cubic_spline_control_point{
					.y = begin_elevation,
					.ddx = -0.5f*dl*begin_ddx  // Divide by two to compensate for spline being compressed
				},
				cubic_spline_control_point{
					.y = col_elvation,
					.ddx = 0.0f
				}
			);

			auto const p_peak_end = make_polynomial(
				cubic_spline_control_point{
					.y = col_elvation,
					.ddx = 0.0f
				},
				cubic_spline_control_point{
					.y = end_elevation,
					.ddx = 0.5f*dl*end_ddx  // Divide by two to compensate for spline being compressed
				}
			);

			auto const mod_func = [p_peak_begin, p_peak_end, col_elvation](auto x) {
				return std::max(x < 0.5f? p_peak_begin(2.0f*x) : p_peak_end(2.0f*(x - 0.5f)), col_elvation);
			};

			for(auto l = begin_index; l != end_index; ++l)
			{
				auto const t = integrated_curve_length[l];
				auto const x = t - integrated_curve_length[begin_index];
				ret[l] = std::max(initial_elevation(t/L), 0.0f)
					*(1.0f + mod_depth*std::lerp(mod_func(x/dl), noise_array[l], peak_noise_mix));
			}

			begin_elevation = end_elevation;
			begin_index = end_index;
		}

		auto const dl = L - integrated_curve_length[begin_index];

		auto const begin_ddx = wrap_derivative(
			std::tan(two_pi*pick(elevation_profile.per_peak_modulation.slope, rng)),
			begin_elevation,
			initial_elevation,
			integrated_curve_length[begin_index],
			L,
			peak_gain
		);

		auto const col_elvation = -peak_elevation_distribution(rng);

		auto const p_peak_final = make_polynomial(
			cubic_spline_control_point{
				.y = begin_elevation,
				.ddx = -dl*begin_ddx  // Divide by two to compensate for spline being compressed
			},
			cubic_spline_control_point{
				.y = col_elvation,
				.ddx = 0.0f
			}
		);

		for(auto l = begin_index; l != std::size(integrated_curve_length); ++l)
		{
			auto const t = integrated_curve_length[l];
			auto const x = t - integrated_curve_length[begin_index];
			ret[l] = std::max(initial_elevation(t/L), 0.0f)
				*(1.0f + mod_depth*std::lerp(p_peak_final(x/dl), noise_array[l], peak_noise_mix));
		}
	}
	return ret;
}

terraformer::ridge_tree_branch_sequence terraformer::generate_branches(
	ridge_tree_branch_seed_sequence const& branch_points,
	span_2d<float const> current_heightmap,
	float pixel_size,
	ridge_tree_branch_displacement_description const& curve_desc,
	random_generator& rng,
	float d_max,
	ridge_tree_branch_sequence&& gen_branches)
{
	auto const points = branch_points.get<0>();
	auto const normals = branch_points.get<1>();
	auto const vertex_index = branch_points.get<2>();

	for(auto k : branch_points.element_indices())
	{
		auto const base_curve = generate_branch_base_curve(
			points[k],
			normals[k],
			current_heightmap,
			pixel_size,
			[
				d_max,
				d = 0.0f,
				loc_prev = points[k]
			](auto loc) mutable {
				auto new_distance = d + distance(loc, loc_prev);
				if(new_distance > d_max)
				{ return true; }

				d = new_distance;
				loc_prev = loc;

				return false;
			}
		);

		if(std::size(base_curve).get() < 3)
		{
			gen_branches.push_back(
				displaced_curve{},
				vertex_index[k],
				single_array<displaced_curve::index_type>{},
				single_array<float>{}
			);
			continue;
		}

		array_size<float> const base_curve_length{static_cast<size_t>(curve_length(base_curve)/pixel_size) + 1};
		auto const offsets = generate(curve_desc, rng, base_curve_length, pixel_size);

		auto displaced_curve = displace_xy(
			base_curve,
			displacement_profile{
				.offsets = offsets,
				.sample_period = pixel_size,
			}
		);

		auto integrated_curve_length = curve_running_length_xy(std::as_const(displaced_curve).points());

		gen_branches.push_back(
			std::move(displaced_curve),
			vertex_index[k],
			single_array<displaced_curve::index_type>{},
			std::move(integrated_curve_length)
		);
	}

	return gen_branches;
}

void terraformer::trim_at_intersect(
	trim_params const& a_params,
	trim_params const& b_params
)
{
	assert(std::size(a_params.collision_margins).get() == std::size(a_params.curves).get());
	assert(std::size(b_params.collision_margins).get() == std::size(b_params.curves).get());

	auto const a = a_params.curves;
	auto const b = b_params.curves;
	auto const a_margins = a_params.collision_margins;
	auto const b_margins = b_params.collision_margins;

	auto const outer_count = std::size(a);
	auto const inner_count = std::size(b);

	// TODO: It would be nice to have different types for a_trim and b_trim
	single_array<displaced_curve::index_type> a_trim(array_size<displaced_curve::index_type>{outer_count});
	for(auto k : a_trim.element_indices())
	{
		array_index<displaced_curve> const src_index{k.get()};
		a_trim[k] = displaced_curve::index_type{std::size(a[src_index])};
	}

	single_array<displaced_curve::index_type> b_trim(array_size<displaced_curve::index_type>{inner_count});
	for(auto l : b_trim.element_indices())
	{
		array_index<displaced_curve> const src_index{l.get()};
		b_trim[l] = displaced_curve::index_type{std::size(b[src_index])};
	}

	for(auto k : a_trim.element_indices())
	{
		for(auto l : b_trim.element_indices())
		{
			array_index<displaced_curve> const src_index_k{k.get()};
			array_index<displaced_curve> const src_index_l{l.get()};
			auto const margin_a = a_margins[array_index<float>{k.get()}];
			auto const margin_b = b_margins[array_index<float>{l.get()}];
			auto const md2 = (margin_a + margin_b)*(margin_a + margin_b);
			auto const res = find_matching_pair(
				a[src_index_k].get<0>(),
				b[src_index_l].get<0>(),
				[md2](auto const p1, auto const p2) {
					auto const d2 = distance_squared(p1, p2);
					if(d2 < md2)
					{	return true; }
					return false;
				}
			);

			a_trim[k] = std::min(
				static_cast<displaced_curve::index_type>(as_index(std::begin(a[src_index_k].get<0>()), res.first)),
				a_trim[k]
			);
			b_trim[l] = std::min(
				static_cast<displaced_curve::index_type>(as_index(std::begin(b[src_index_l].get<0>()), res.second)),
				b_trim[l]
			);
		}
	}

	for(auto k : a_trim.element_indices())
	{
		for(auto l = a_trim.element_indices().front(); l != k; ++l)
		{
			array_index<displaced_curve> const src_index_k{k.get()};
			array_index<displaced_curve> const src_index_l{l.get()};
			auto const margin_ak = a_margins[array_index<float>{k.get()}];
			auto const margin_al= a_margins[array_index<float>{l.get()}];
			auto const md2 = (margin_ak + margin_al)*(margin_ak + margin_al);
			auto const res = find_matching_pair(
				a[src_index_k].get<0>(),
				a[src_index_l].get<0>(),
				[md2](auto const p1, auto const p2) {
					if(distance_squared(p1, p2) < md2)
					{ return true; }
					return false;
				}
			);

			a_trim[k] = std::min(
				static_cast<displaced_curve::index_type>(as_index(std::begin(a[src_index_k].get<0>()), res.first)),
				a_trim[k]
			);

			a_trim[l] = std::min(
				static_cast<displaced_curve::index_type>(as_index(std::begin(a[src_index_l].get<0>()), res.second)),
				a_trim[l]
			);
		}
	}

	for(auto k : b_trim.element_indices())
	{
		for(auto l = b_trim.element_indices().front(); l != k; ++l)
		{
			array_index<displaced_curve> const src_index_k{k.get()};
			array_index<displaced_curve> const src_index_l{l.get()};
			auto const margin_bk = b_margins[array_index<float>{k.get()}];
			auto const margin_bl= b_margins[array_index<float>{l.get()}];
			auto const md2 = (margin_bk + margin_bl)*(margin_bk + margin_bl);

			auto const res = find_matching_pair(
				b[src_index_k].get<0>(),
				b[src_index_l].get<0>(),
				[md2](auto const p1, auto const p2) {
					if(distance_squared(p1, p2) < md2)
					{ return true; }
					return false;
				}
			);

			b_trim[k] = std::min(
				static_cast<displaced_curve::index_type>(as_index(std::begin(b[src_index_k].get<0>()), res.first)),
				b_trim[k]
			);
			b_trim[l] = std::min(
				static_cast<displaced_curve::index_type>(as_index(std::begin(b[src_index_l].get<0>()), res.second)),
				b_trim[l]
			);
		}
	}

	for(auto k : a_trim.element_indices())
	{
		auto const index = a_trim[k];
		array_index<displaced_curve> const src_index_k{k.get()};
		if(index != std::size(a[src_index_k]))
		{ a[src_index_k].truncate_from(index); }
	}

	for(auto l : b_trim.element_indices())
	{
		auto const index = b_trim[l];
		array_index<displaced_curve> const src_index_l{l.get()};
		if(index != std::size(b[src_index_l]))
		{ b[src_index_l].truncate_from(index); }
	}
}

terraformer::single_array<terraformer::ridge_tree_stem_collection>
terraformer::generate_branches(
	std::span<ridge_tree_branch_seed_sequence_pair const> parents,
	span_2d<float const> current_heightmap,
	float pixel_size,
	ridge_tree_branch_displacement_description const& curve_desc,
	random_generator& rng,
	ridge_tree_branch_growth_description growth_params
)
{
	single_array<ridge_tree_stem_collection> ret;

	if(parents.empty())
	{	return ret; }

	ridge_tree_stem_collection current_stem_collection{array_index<displaced_curve>{0}};
	current_stem_collection.left = generate_branches(
		parents[0].left,
		current_heightmap,
		pixel_size,
		curve_desc,
		rng,
		growth_params.max_length
	);

	trim_params dummy{};
	trim_at_intersect(
		trim_params{
			.curves = current_stem_collection.left.get<0>(),
			.collision_margins = parents[0].left.get<3>()
		},
		dummy
	);

	for(size_t k = 1; k != std::size(parents); ++k)
	{
		current_stem_collection.right = generate_branches(
			parents[k - 1].right,
			current_heightmap,
			pixel_size,
			curve_desc,
			rng,
			growth_params.max_length
		);


		auto left_branches = generate_branches(
			parents[k].left,
			current_heightmap,
			pixel_size,
			curve_desc,
			rng,
			growth_params.max_length
		);

		trim_at_intersect(
			trim_params{
				.curves = current_stem_collection.right.get<0>(),
				.collision_margins = parents[k - 1].right.get<3>()
			},
			trim_params{
				.curves = left_branches.get<0>(),
				.collision_margins = parents[k].left.get<3>()
			}
		);
		ret.push_back(std::move(current_stem_collection));

		current_stem_collection = ridge_tree_stem_collection{array_index<displaced_curve>{k}};
		current_stem_collection.left = std::move(left_branches);
	}

	current_stem_collection.right = generate_branches(
		parents.back().right,
		current_heightmap,
		pixel_size,
		curve_desc,
		rng,
		growth_params.max_length
	);
	trim_at_intersect(
		trim_params{
			.curves = current_stem_collection.right.get<0>(),
			.collision_margins = parents.back().right.get<3>()
		},
		dummy
	);

	ret.push_back(std::move(current_stem_collection));
	return ret;
}