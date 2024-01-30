//@	{"target": {"name":"./ridge_tree_branch.o"}}

#include "./ridge_tree_branch.hpp"

terraformer::displacement terraformer::compute_field(std::span<displaced_curve const> branches, location r, float min_distance)
{
	displacement ret{};

	for(size_t k = 0; k != std::size(branches); ++k)
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

terraformer::displacement terraformer::compute_field(std::span<ridge_tree_branch_collection const> branches, location r, float min_distance)
{
	displacement ret{};

	for(size_t k = 0; k != std::size(branches); ++k)
	{ ret += compute_field(branches[k].curves.get<0>(), r, min_distance); }

	return ret;
}

terraformer::array_tuple<terraformer::displaced_curve, size_t>
terraformer::generate_branches(
	array_tuple<location, direction, size_t> const& branch_points,
	std::span<ridge_tree_branch_collection const> existing_branches,
	float pixel_size,
	ridge_tree_branch_displacement_description curve_desc,
	random_generator& rng,
	float d_max,
	array_tuple<displaced_curve, size_t>&& gen_branches)
{
	auto const points = branch_points.get<0>();
	auto const normals = branch_points.get<1>();
	auto const branch_index = branch_points.get<2>();
	for(size_t k = 0; k != std::size(branch_points); ++k)
	{
		auto const base_curve = generate_branch_base_curve(
			points[k],
			normals[k],
			existing_branches,
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

		if(std::size(base_curve) < 3)
		{
			fprintf(stderr, "Curve %zu is to short\n", k);
			continue;
		}

		auto const base_curve_length = static_cast<size_t>(curve_length(base_curve)/pixel_size) + 1;
		auto const offsets = generate(curve_desc, rng, base_curve_length, pixel_size);

		gen_branches.push_back(
			tuple{
				displace_xy(
					base_curve,
					displacement_profile{
						.offsets = offsets,
						.sample_period = pixel_size,
					}
				),
				branch_index[k]
			}
		);
	}

	return gen_branches;
}

void terraformer::trim_at_intersect(std::span<displaced_curve> a, std::span<displaced_curve> b, float min_distance)
{
	auto const md2 = min_distance*min_distance;

	auto const outer_count = std::size(a);
	auto const inner_count = std::size(b);

	std::vector<size_t> a_trim(outer_count);
	for(size_t k = 0; k != outer_count; ++k)
	{ a_trim[k] = std::size(a[k]); }

	std::vector<size_t> b_trim(inner_count);
	for(size_t l = 0; l != inner_count; ++l)
	{ b_trim[l] = std::size(b[l]); }

	for(size_t k = 0; k != outer_count; ++k)
	{
		for(size_t l = 0; l != inner_count; ++l)
		{
			auto const res = cartesian_find_if(
				std::span<location const>(a[k].get<0>()),
				std::span<location const>(b[l].get<0>()),
				[md2](auto const p1, auto const p2) {
					if(distance_squared(p1, p2) < md2)
					{ return true; }
					return false;
				}
			);

			a_trim[k] = std::min(res.first, a_trim[k]);
			b_trim[l] = std::min(res.second, b_trim[l]);
		}
	}
	for(size_t k = 0; k != outer_count; ++k)
	{
		for(size_t l = 0; l != k; ++l)
		{
			auto const res = cartesian_find_if(
				std::span<location const>(a[k].get<0>()),
				std::span<location const>(a[l].get<0>()),
				[md2](auto const p1, auto const p2) {
					if(distance_squared(p1, p2) < md2)
					{ return true; }
					return false;
				}
			);

			a_trim[k] = std::min(res.first, a_trim[k]);
			a_trim[l] = std::min(res.second, a_trim[l]);
		}
	}

	for(size_t k = 0; k != inner_count; ++k)
	{
		for(size_t l = 0; l != k; ++l)
		{
			auto const res = cartesian_find_if(
				std::span<location const>(b[k].get<0>()),
				std::span<location const>(b[l].get<0>()),
				[md2](auto const p1, auto const p2) {
					if(distance_squared(p1, p2) < md2)
					{ return true; }
					return false;
				}
			);

			b_trim[k] = std::min(res.first, b_trim[k]);
			b_trim[l] = std::min(res.second, b_trim[l]);
		}
	}

	for(size_t k = 0; k != outer_count; ++k)
	{
		auto const index = a_trim[k];
		if(index != std::size(a[k]))
		{ a[k].shrink(index); }
	}

	for(size_t l = 0; l != inner_count; ++l)
	{
		auto const index = b_trim[l];
		if(index != std::size(b[l]))
		{	b[l].shrink(index); }
	}
}

std::vector<terraformer::ridge_tree_stem_collection>
terraformer::generate_branches(
	std::span<ridge_tree_branch_seed_sequence const> parents,
	std::span<ridge_tree_branch_collection const> existing_branches,
	float pixel_size,
	ridge_tree_branch_displacement_description curve_desc,
	random_generator& rng,
	ridge_tree_branch_growth_description growth_params
)
{
	std::vector<ridge_tree_stem_collection> ret;

	if(std::size(parents) == 0)
	{	return ret; }

	ridge_tree_stem_collection current_stem_collection;
	current_stem_collection.left = generate_branches(
		parents[0].left,
		existing_branches,
		pixel_size,
		curve_desc,
		rng,
		growth_params.max_length
	);
	std::span<displaced_curve> dummy{};
	trim_at_intersect(current_stem_collection.left.get<0>(), dummy, growth_params.min_neighbour_distance);

	for(size_t k = 1; k != std::size(parents); ++k)
	{
		current_stem_collection.right = generate_branches(
			parents[k - 1].right,
			existing_branches,
			pixel_size,
			curve_desc,
			rng,
			growth_params.max_length
		);

		auto left_branches = generate_branches(
			parents[k].left,
			existing_branches,
			pixel_size,
			curve_desc,
			rng,
			growth_params.max_length
		);

		trim_at_intersect(current_stem_collection.right.get<0>(), left_branches.get<0>(), growth_params.min_neighbour_distance);
		ret.push_back(std::move(current_stem_collection));
		current_stem_collection.left = std::move(left_branches);
	}

	current_stem_collection.right = generate_branches(
		parents.back().right,
		existing_branches,
		pixel_size,
		curve_desc,
		rng,
		growth_params.max_length
	);
	trim_at_intersect(current_stem_collection.right.get<0>(), dummy, growth_params.min_neighbour_distance);

	ret.push_back(std::move(current_stem_collection));
	return ret;
}