//@	{"target": {"name":"./ridge_tree_branch.o"}}

#include "./ridge_tree_branch.hpp"

#include "lib/curve_tools/length.hpp"

#include <random>

terraformer::polynomial<3> terraformer::create_polynomial(
	float curve_length,
	elevation z_0,
	ridge_elevation_profile_description const& elevation_profile,
	random_generator& rng
)
{
	std::uniform_real_distribution slope_distribution{0.0f, 1.0f};
	auto const t_begin = slope_distribution(rng);
	auto const begin_range = elevation_profile.starting_slope.max
		- elevation_profile.starting_slope.min;

	auto const t_end = slope_distribution(rng);
	auto const end_range = elevation_profile.final_slope.max
		- elevation_profile.final_slope.min;

	constexpr auto two_pi = std::numbers::pi_v<float>;
	return make_polynomial(
		cubic_spline_control_point{
			.y = z_0,
			.ddx = -curve_length*std::tan(two_pi*(elevation_profile.starting_slope.min + t_begin*begin_range))
		},
		cubic_spline_control_point{
			.y = elevation_profile.final_elevation,
			.ddx = -curve_length*std::tan(two_pi*(elevation_profile.final_slope.min + t_end*end_range))
		}
	);
}

terraformer::single_array<float> terraformer::generate_elevation_profile(
	span<float const, array_index<float>, array_size<float>> integrated_curve_length,
	polynomial<3> const& ridge_polynomial
)
{
	if(integrated_curve_length.empty())
	{ return terraformer::single_array<float>{}; }

	auto const L = integrated_curve_length.back();

	single_array ret{std::size(integrated_curve_length)};
	for(auto k = ret.first_element_index(); k != std::size(ret); ++k)
	{ ret[k] = ridge_polynomial(integrated_curve_length[k]/L); }

	return ret;
}

terraformer::single_array<float> terraformer::generate_elevation_profile(
	span<float const> integrated_curve_length,
	span<displaced_curve::index_type const> branch_points,
	polynomial<3> const& initial_curve,
	peak_elevation_description const& elevation_profile,
	random_generator& rng
)
{
	// TODO: first element should use the post-modulated value from the parent as begin_elevation

	if(integrated_curve_length.empty())
	{ return terraformer::single_array<float>{}; }

	auto const L = integrated_curve_length.back();
	single_array ret{std::size(integrated_curve_length)};
	auto begin_elevation = 0.0f;
	auto begin_index = integrated_curve_length.first_element_index();
	std::uniform_real_distribution peak_elevation_distribution{0.0f, 1.0f};
	for(auto k = branch_points.first_element_index();
		k != std::size(branch_points);
		++k
	)
	{
		array_index<float> const end_index{branch_points[k].get()};
		auto const dl = integrated_curve_length[end_index] - integrated_curve_length[begin_index];
		auto const end_elevation = peak_elevation_distribution(rng);
		auto const col_elvation = -peak_elevation_distribution(rng);
		auto const p_peak_begin = make_polynomial(
			cubic_spline_control_point{
				.y = begin_elevation,
				.ddx = 0.0f  // TODO
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
				.ddx = 0.0f // TODO
			}
		);

		auto const mod_func = [p_peak_begin, p_peak_end](auto x) {
			return x < 0.5f? p_peak_begin(2.0f*x) : p_peak_end(2.0f*(x - 0.5f));
		};

		auto const mod_depth = elevation_profile.mod_depth;
		for(auto l = begin_index; l != end_index; ++l)
		{
			auto const t = integrated_curve_length[l];
			auto const x = t - integrated_curve_length[begin_index];
			ret[l] = initial_curve(t/L)*(1.0f + mod_depth*mod_func(x/dl));
		}

		begin_elevation = end_elevation;
		begin_index = end_index;
	}

	for(auto l = begin_index; l != std::size(integrated_curve_length); ++l)
	{
		auto const t = integrated_curve_length[l];
		ret[l] = initial_curve(t/L);
	}

	return ret;
}

terraformer::displacement terraformer::compute_field(span<displaced_curve const> branches, location r, float min_distance)
{
	displacement ret{};

	for(auto k = branches.first_element_index(); k != std::size(branches); ++k)
	{
		auto const points = branches[k].get<0>();
		ret += terraformer::fold_over_line_segments(
			points,
			[](auto seg, auto point, auto d02, auto... prev) {
				auto const p = closest_point(seg, point);
				auto const d2 = distance_squared(p, point);
				direction const r{p - point};
				auto const l = length(seg);
				return (prev + ... + (l*r*(d2<d02? 1.0f : d02/d2)));
			},
			r,
			min_distance*min_distance
		);
	}

	return ret;
}

terraformer::displacement terraformer::compute_field(
	span<ridge_tree_trunk const> branch_infos,
	location r,
	float min_distance
)
{
	displacement ret{};

	for(auto k = branch_infos.first_element_index(); k != std::size(branch_infos); ++k)
	{ ret += compute_field(branch_infos[k].branches.get<0>(), r, min_distance); }

	return ret;
}

terraformer::ridge_tree_branch_sequence terraformer::generate_branches(
	ridge_tree_branch_seed_sequence const& branch_points,
	span<ridge_tree_trunk const> trunks,
	float pixel_size,
	ridge_tree_branch_displacement_description curve_desc,
	random_generator& rng,
	float d_max,
	ridge_tree_branch_sequence&& gen_branches)
{
	auto const points = branch_points.get<0>();
	auto const normals = branch_points.get<1>();
	auto const vertex_index = branch_points.get<2>();

	for(auto k = branch_points.first_element_index(); k != std::size(branch_points); ++k)
	{
		auto const base_curve = generate_branch_base_curve(
			points[k],
			normals[k],
			trunks,
			pixel_size,
			[d = 0.0f, loc_prev = points[k], d_max](auto loc) mutable {
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
			fprintf(stderr, "Curve %zu is to short\n", k.get());
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

void terraformer::trim_at_intersect(span<displaced_curve> a, span<displaced_curve> b, float min_distance)
{
	auto const md2 = min_distance*min_distance;

	auto const outer_count = std::size(a);
	auto const inner_count = std::size(b);

	// TODO: It would be nice to have different types for a_trim and b_trim
	single_array<displaced_curve::index_type> a_trim(array_size<displaced_curve::index_type>{outer_count});
	for(auto k = a_trim.first_element_index(); k != std::size(a_trim); ++k)
	{
		array_index<displaced_curve> const src_index{k.get()};
		a_trim[k] = displaced_curve::index_type{std::size(a[src_index])};
	}

	single_array<displaced_curve::index_type> b_trim(array_size<displaced_curve::index_type>{inner_count});
	for(auto l = b_trim.first_element_index(); l != std::size(b_trim); ++l)
	{
		array_index<displaced_curve> const src_index{l.get()};
		b_trim[l] = displaced_curve::index_type{std::size(b[src_index])};
	}

	for(auto k = a_trim.first_element_index(); k != std::size(a_trim); ++k)
	{
		for(auto l = b_trim.first_element_index(); l != std::size(b_trim); ++l)
		{
			array_index<displaced_curve> const src_index_k{k.get()};
			array_index<displaced_curve> const src_index_l{l.get()};
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

	for(auto k = a_trim.first_element_index(); k != std::size(a_trim); ++k)
	{
		for(auto l = a_trim.first_element_index(); l != k; ++l)
		{
			array_index<displaced_curve> const src_index_k{k.get()};
			array_index<displaced_curve> const src_index_l{l.get()};

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

	for(auto k = b_trim.first_element_index(); k != std::size(b_trim); ++k)
	{
		for(auto l = b_trim.first_element_index(); l != k; ++l)
		{
			array_index<displaced_curve> const src_index_k{k.get()};
			array_index<displaced_curve> const src_index_l{l.get()};

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
	for(auto k = a_trim.first_element_index(); k != std::size(a_trim); ++k)
	{
		auto const index = a_trim[k];
		array_index<displaced_curve> const src_index_k{k.get()};
		if(index != std::size(a[src_index_k]))
		{ a[src_index_k].truncate_from(index); }
	}

	for(auto l = b_trim.first_element_index(); l != std::size(b_trim); ++l)
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
	span<ridge_tree_trunk const> trunks,
	float pixel_size,
	ridge_tree_branch_displacement_description curve_desc,
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
		trunks,
		pixel_size,
		curve_desc,
		rng,
		growth_params.max_length
	);
	span<displaced_curve> dummy{};
	trim_at_intersect(current_stem_collection.left.get<0>(), dummy, growth_params.min_neighbour_distance);

	for(size_t k = 1; k != std::size(parents); ++k)
	{
		current_stem_collection.right = generate_branches(
			parents[k - 1].right,
			trunks,
			pixel_size,
			curve_desc,
			rng,
			growth_params.max_length
		);


		auto left_branches = generate_branches(
			parents[k].left,
			trunks,
			pixel_size,
			curve_desc,
			rng,
			growth_params.max_length
		);

		trim_at_intersect(current_stem_collection.right.get<0>(), left_branches.get<0>(), growth_params.min_neighbour_distance);
		ret.push_back(std::move(current_stem_collection));

		current_stem_collection = ridge_tree_stem_collection{array_index<displaced_curve>{k}};
		current_stem_collection.left = std::move(left_branches);
	}

	current_stem_collection.right = generate_branches(
		parents.back().right,
		trunks,
		pixel_size,
		curve_desc,
		rng,
		growth_params.max_length
	);
	trim_at_intersect(current_stem_collection.right.get<0>(), dummy, growth_params.min_neighbour_distance);

	ret.push_back(std::move(current_stem_collection));
	return ret;
}