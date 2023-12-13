//@	{"target":{"name":"curve_displace.o"}}

#include "./curve_displace.hpp"
#include "./interp.hpp"
#include "./boundary_sampling_policies.hpp"

#include <cassert>

terraformer::array_tuple<terraformer::location, float>  terraformer::displace(std::span<location const> c, displacement_profile dy, displacement looking_towards)
{
	assert(std::size(c) >= 3);
	auto c_distance = 0.0f;
	terraformer::array_tuple<terraformer::location, float>  ret(std::size(c));
	auto points = ret.get<0>();
	auto offsets = ret.get<1>();

	{
		auto const k = 1;
		auto const val = dy.offsets[0];
		auto const n = curve_vertex_normal_from_projection(c[k - 1], c[k], c[k + 1], looking_towards);
		points[k - 1] = c[k - 1] + val*n;
		offsets[k - 1] = val;
	}

	for(size_t k = 1; k != std::size(c) - 1; ++k)
	{
		c_distance += distance(c[k], c[k - 1]);
		auto const sample_at = c_distance/dy.sample_period;
		auto const val = interp(dy.offsets, sample_at, clamp_at_boundary{});
		auto const n = curve_vertex_normal_from_projection(c[k - 1], c[k], c[k + 1], looking_towards);
		points[k] = c[k] + val*n;
		offsets[k] = val;
	}

	{
		auto const k = std::size(c) - 2;
		c_distance += distance(c[k + 1], c[k]);
		auto const sample_at = c_distance/dy.sample_period;
		auto const val = interp(dy.offsets, sample_at, clamp_at_boundary{});
		auto const n = curve_vertex_normal_from_projection(c[k - 1], c[k], c[k + 1], looking_towards);
		points[k + 1] = c[k + 1] + val*n;
		offsets[k] = val;
	}

	return ret;
}
