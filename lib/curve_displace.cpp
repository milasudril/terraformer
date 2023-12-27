//@	{"target":{"name":"curve_displace.o"}}

#include "./curve_displace.hpp"
#include "./interp.hpp"
#include "./boundary_sampling_policies.hpp"

#include "./rng.hpp"
#include <random>

#include <cassert>

terraformer::array_tuple<terraformer::location, float>  terraformer::displace(std::span<location const> c, displacement_profile dy, displacement looking_towards)
{
	assert(std::size(c) >= 3);
	auto c_distance = 0.0f;
	terraformer::array_tuple<terraformer::location, float>  ret(std::size(c));
	auto points = ret.get<0>();
	auto offsets = ret.get<1>();

	random_generator rng;
	std::uniform_real_distribution U{-1.0f, 1.0f};

	{
		auto const k = 1;
		auto const val = dy.offsets[0];
		auto mid = c[k];
		auto n = curve_vertex_normal_from_projection(c[k - 1], mid, c[k + 1], looking_towards);
		if(isnan(n[0])) [[unlikely]]
		{
			mid += distance(c[k], c[k - 1])*direction{displacement{U(rng), U(rng), U(rng)}}/1024.0f;
			n = curve_vertex_normal_from_projection(c[k - 1], mid, c[k + 1], looking_towards);
		}

		points[k - 1] = c[k - 1] + val*n;
		offsets[k - 1] = val;
	}

	for(size_t k = 1; k != std::size(c) - 1; ++k)
	{
		auto const d = distance(c[k], c[k - 1]);
		c_distance += d;
		auto const sample_at = c_distance/dy.sample_period;
		auto const val = interp(dy.offsets, sample_at, clamp_at_boundary{});
		auto mid = c[k];
		auto n = curve_vertex_normal_from_projection(c[k - 1], mid, c[k + 1], looking_towards);
		if(isnan(n[0])) [[unlikely]]
		{
			mid += distance(c[k], c[k - 1])*direction{displacement{U(rng), U(rng), U(rng)}}/1024.0f;
			n = curve_vertex_normal_from_projection(c[k - 1], mid, c[k + 1], looking_towards);
		}

		points[k] = c[k] + val*n;
		offsets[k] = val;
	}

	{
		auto const k = std::size(c) - 2;
		auto const d = distance(c[k], c[k - 1]);
		c_distance += d;
		auto const sample_at = c_distance/dy.sample_period;
		auto const val = interp(dy.offsets, sample_at, clamp_at_boundary{});
		auto mid = c[k];
		auto n = curve_vertex_normal_from_projection(c[k - 1], mid, c[k + 1], looking_towards);
		if(isnan(n[0])) [[unlikely]]
		{
			mid += distance(c[k], c[k - 1])*direction{displacement{U(rng), U(rng), U(rng)}}/1024.0f;
			n = curve_vertex_normal_from_projection(c[k - 1], mid, c[k + 1], looking_towards);
		}

		points[k + 1] = c[k + 1] + val*n;
		offsets[k] = val;
	}

	return ret;
}
