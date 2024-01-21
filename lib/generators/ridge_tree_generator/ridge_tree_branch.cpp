//@	{"target": {"name":"./ridge_tree_branch.o"}}

#include "./ridge_tree_branch.hpp"

std::vector<terraformer::ridge_tree_branch>
terraformer::generate_branches(
	array_tuple<location, direction> const& branch_points,
	span_2d<float const> potential,
	float pixel_size,
	ridge_curve_description curve_desc,
	random_generator& rng,
	float d_max,
	std::vector<ridge_tree_branch>&& existing_branches)
{
	auto const points = branch_points.get<0>();
	auto const normals = branch_points.get<1>();
	for(size_t k = 0; k != std::size(branch_points); ++k)
	{
		auto const base_curve = generate_branch_base_curve(
			points[k],
			normals[k],
			potential,
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

		existing_branches.push_back(
			displace_xy(
				base_curve,
				displacement_profile{
					.offsets = offsets,
					.sample_period = pixel_size,
				}
			)
		);
	}

	return existing_branches;
}

void terraformer::trim_at_intersect(std::vector<ridge_tree_branch>& a, std::vector<ridge_tree_branch>& b, float min_distance)
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

std::vector<terraformer::ridge_tree_stem>
terraformer::generate_branches(
	std::span<ridge_tree_branch_seed_sequence const> parents,
	span_2d<float const> potential,
	float pixel_size,
	ridge_curve_description curve_desc,
	random_generator& rng,
	float max_length
)
{
	if(std::size(parents) == 0)
	{	return std::vector<ridge_tree_stem>{}; }

	std::vector<ridge_tree_stem> ret;

	ridge_tree_stem current_stem;
	current_stem.left = generate_branches(
		parents[0].right,
		potential,
		pixel_size,
		curve_desc,
		rng,
		max_length
	);

	for(size_t k = 1; k != std::size(parents); ++k)
	{
		printf("%.8g  %.8g\n", parents[k - 1].left.get<0>()[0][0], parents[k].right.get<0>()[0][0]);

		current_stem.right = generate_branches(
			parents[k - 1].left,
			potential,
			pixel_size,
			curve_desc,
			rng,
			max_length
		);

		auto left_branches = generate_branches(
			parents[k].right,
			potential,
			pixel_size,
			curve_desc,
			rng,
			max_length
		);

		trim_at_intersect(current_stem.right, left_branches, 1536.0f);  // TODO: Need to pass the min distance
		ret.push_back(std::move(current_stem));
		current_stem.left = std::move(left_branches);
	}

	ret.back().right = generate_branches(
		parents.back().right,
		potential,
		pixel_size,
		curve_desc,
		rng,
		max_length
	);

	printf("\n");
	return ret;
}

float terraformer::compute_potential(std::span<ridge_tree_branch const> branches, location r, float min_distance)
{
	auto sum = 0.0f;
	for(size_t k = 0; k != std::size(branches); ++k)
	{
		auto const points = branches[k].get<0>();
		sum += terraformer::fold_over_line_segments(
			points,
			[](auto seg, auto point, auto d02, auto... prev) {
				auto const d2 = distance_squared(seg, point);
				auto const l = length(seg);
				return (prev + ... + (l*(d2<d02? 1.0f : d02/d2)));
			},
			r,
			min_distance*min_distance
		);
	}
	return sum;
}

void terraformer::compute_potential(
	span_2d<float> output,
	std::span<ridge_tree_branch const> left_siblings,
	std::span<ridge_tree_branch const> right_siblings,
	float pixel_size)
{
	for(uint32_t y = 0; y != output.height(); ++y)
	{
		for(uint32_t x = 0; x != output.width(); ++x)
		{
			auto sum = 0.0f;
			terraformer::location const loc_xy{
				pixel_size*static_cast<float>(x),
				pixel_size*static_cast<float>(y),
				0.0f
			};

			sum += terraformer::compute_potential(left_siblings, loc_xy, 0.5f*pixel_size);
			sum += terraformer::compute_potential(right_siblings, loc_xy, 0.5f*pixel_size);

			output(x, y) += sum;
		}
	}
}