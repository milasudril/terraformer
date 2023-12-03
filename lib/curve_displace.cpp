//@	{"target":{"name":"curve_displace.o"}}

#include "./curve_displace.hpp"
#include "./interp.hpp"
#include "./boundary_sampling_policies.hpp"

#include <cassert>

std::vector<terraformer::location> terraformer::displace(std::span<location const> c, displacement_profile dy, displacement looking_towards)
{
	assert(std::size(c) >= 3);
	auto c_distance = 0.0f;
	std::vector<location> ret(std::size(c));

	{
		auto const k = 1;
		auto const val = dy.offsets[0];
		auto const n = curve_vertex_normal_from_projection(c[k - 1], c[k], c[k + 1], looking_towards);
		ret[k - 1] = c[k - 1] + val*n;
	}

	for(size_t k = 1; k != std::size(c) - 1; ++k)
	{
		c_distance += distance(c[k], c[k - 1]);
		auto const sample_at = c_distance/dy.sample_period;
		auto const val = interp(dy.offsets, sample_at, clamp_at_boundary{});
		auto const n = curve_vertex_normal_from_projection(c[k - 1], c[k], c[k + 1], looking_towards);
		ret[k] = c[k] + val*n;
	}

	{
		auto const k = std::size(c) - 2;
		c_distance += distance(c[k + 1], c[k]);
		auto const sample_at = c_distance/dy.sample_period;
		auto const val = interp(dy.offsets, sample_at, clamp_at_boundary{});
		auto const n = curve_vertex_normal_from_projection(c[k - 1], c[k], c[k + 1], looking_towards);
		ret[k + 1] = c[k + 1] + val*n;
	}

	return ret;
}
