//@	{"target": {"name":"./ridge_tree_branch.o"}}

#include "./ridge_tree_branch.hpp"

#include "lib/curve_tools/length.hpp"

#include <random>


terraformer::single_array<float> terraformer::generate_elevation_profile(
	span<location const> branch_curve,
	float initial_elevation,
	ridge_elevation_profile_description const& ridge_elevation_profile
#if 0
	span<array_index<location> const> branch_points,
	peak_elevation_description const& peak_elvation_profile,
	random_generator& rng
#endif
)
{
	auto const running_length = curve_running_length_xy(branch_curve);
	if(running_length.empty())
	{ return running_length; }

	auto const L = running_length.back();

	constexpr auto two_pi = std::numbers::pi_v<float>;

	auto const p_ridge = make_polynomial(
		cubic_spline_control_point{
			.y = initial_elevation,
			.ddx = L*std::tan(two_pi*ridge_elevation_profile.starting_slope)
		},
		cubic_spline_control_point{
			.y = ridge_elevation_profile.final_elevation,
			.ddx = L*std::tan(two_pi*ridge_elevation_profile.final_slope)
		}
	);

	single_array ret{std::size(running_length)};
	for(auto k = ret.first_element_index(); k != std::size(ret); ++k)
	{	ret[k] = p_ridge(running_length[k]/L);	}

#if 0
	auto const p_ridge_deriv = p_ridge.derivative();
	auto const ridge_elevation_deriv = p_ridge_deriv(l/L)/L;
#endif

	return ret;
}


#if 0

terraformer::single_array<float> terraformer::gen_per_branch_point_control_points(
	span<location const, array_index<location>, array_size<location>> locations,
	span<array_index<location> const> branch_points,
	per_branch_point_elevation_profile const& params,
	random_generator& rng
)
{
	single_array<float> ret;
	if(branch_points.empty())
	{ return ret; }

	if(locations.empty())
	{ return ret; }

	std::uniform_real_distribution peak_elevation_distribution{
		0.0f,
		static_cast<float>(params.peak_modulation_depth)
	};

	std::uniform_real_distribution peak_angle_distribution{
		static_cast<float>(params.min_peak_angle),
		static_cast<float>(params.max_peak_angle)
	};

	auto last_value = 0.0f;
	auto last_branch_point = locations.first_element_index();
	auto seg_length = 0.0f;
	auto next_branch_point_index = branch_points.first_element_index();

	for(auto k = locations.first_element_index() + 1; k!= std::size(locations); ++k)
	{
		if(next_branch_point_index == std::size(branch_points))
		{ return ret; }

		seg_length += distance_xy(locations[k], locations[k - 1]);

		if(k == branch_points[next_branch_point_index])
		{
			constexpr auto two_pi = 2.0f*std::numbers::pi_v<float>;
			cubic_spline_control_point const seg_begin{
				.y = last_value,
				.ddx = -std::tan(two_pi*peak_angle_distribution(rng))*seg_length
			};

			auto const value = peak_elevation_distribution(rng);
			cubic_spline_control_point const seg_end{
				.y = value,
				.ddx = std::tan(two_pi*peak_angle_distribution(rng))*seg_length
			};

			last_value = value;

			auto t = 0.0f;
			ret.push_back(interp(seg_begin, seg_end, t));
			for(auto l = last_branch_point + 1; l != k; ++l)
			{
				t += distance_xy(locations[l], locations[l - 1]);
				ret.push_back(interp(seg_begin, seg_end, t));
			}

			seg_length = 0.0f;
			last_branch_point = k;
			++next_branch_point_index;
		}
	}

	return ret;
}
#endif

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

		gen_branches.push_back(
			displace_xy(
				base_curve,
				displacement_profile{
					.offsets = offsets,
					.sample_period = pixel_size,
				}
			),
			vertex_index[k],
			single_array<displaced_curve::index_type>{}
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