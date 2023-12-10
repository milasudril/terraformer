//@	{"dependencies_extra":[{"ref":"./curve_displace.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_CURVE_DISPLACE_HPP
#define TERRAFORMER_CURVE_DISPLACE_HPP

#include "./interp.hpp"
#include "lib/common/spaces.hpp"

#include <vector>
#include <span>

namespace terraformer
{
	inline direction curve_vertex_normal_from_curvature(
		location a,
		location b,
		location c)
	{
		auto const tangent = c - a;
		auto const binormal = cross(tangent, b - a);
		return direction{cross(binormal, tangent)};
	}

	inline direction curve_vertex_normal_from_projection(
		location a,
		location b,
		location c,
		displacement looking_towards)
	{
		auto const tangent = c - a;
		auto const binormal = cross(tangent, b - a);
		auto const side = inner_product(looking_towards, binormal) > 0.0f ? 1.0f : -1.0f;
		return direction{side*cross(binormal, tangent)};
	}

	struct displacement_profile
	{
		std::span<float const> offsets;
		float sample_period;
	};

	std::vector<location> displace(std::span<location const> c, displacement_profile dy, displacement looking_towards);
}

#endif