//@	{"target":{"name":"displace.o"}}

#include "./displace.hpp"

#include "lib/math_utils/boundary_sampling_policies.hpp"
#include "lib/math_utils/interp.hpp"

#include <cassert>

terraformer::displaced_curve terraformer::displace_xy(std::span<location const> c, displacement_profile dy)
{
	assert(std::size(c) >= 3);
	auto c_distance = 0.0f;
	displaced_curve ret(displaced_curve::size_type{std::size(c)});
	auto points = ret.get<0>();
	auto offsets = ret.get<1>();
	auto input_points = ret.get<2>();
	std::copy(std::begin(c), std::end(c), std::begin(input_points));

	{
		auto const k = 1;
		auto const val = dy.offsets[0];
		auto const t = c[k] - c[k - 1];
		// TODO: This operation should be optimized in geosimd
		auto const n = direction{displacement{t[1], -t[0], 0.0f}};
		displaced_curve::index_type const output_index{k - 1};
		points[output_index] = c[k - 1] + val*n;
		offsets[output_index] = val;
	}

	for(size_t k = 1; k != std::size(c) - 1; ++k)
	{
		auto const d = distance_xy(c[k], c[k - 1]);
		c_distance += d;
		auto const sample_at = c_distance/dy.sample_period;
		auto const val = interp_eqdist(dy.offsets, sample_at, clamp_at_boundary{});
		auto const t = c[k + 1] - c[k - 1];
		auto const n = direction{displacement{t[1], -t[0], 0.0f}};
		displaced_curve::index_type const output_index{k};
		points[output_index] = c[k] + val*n;
		offsets[output_index] = val;
	}

	{
		auto const k = std::size(c) - 1;
		auto const d = distance_xy(c[k], c[k - 1]);
		c_distance += d;
		auto const sample_at = c_distance/dy.sample_period;
		auto const val = interp_eqdist(dy.offsets, sample_at, clamp_at_boundary{});
		auto const t = c[k] - c[k - 1];
		auto const n = direction{displacement{t[1], -t[0], 0.0f}};
		displaced_curve::index_type const output_index{k};
		points[output_index] = c[k] + val*n;
		offsets[output_index] = val;
	}

	return ret;
}
