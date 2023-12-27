//@	{"dependencies_extra":[{"ref":"./curve_displace.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_CURVE_DISPLACE_HPP
#define TERRAFORMER_CURVE_DISPLACE_HPP

#include "lib/common/spaces.hpp"
#include "lib/common/array_tuple.hpp"

#include <span>
#include <bit>

namespace terraformer
{
	constexpr bool isnan(float f)
	{
		auto x = std::bit_cast<uint32_t>(f);
		auto pattern = 0x7fc0'0000u;
		return (x&pattern) == pattern ;
	}

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

 array_tuple<location, float> displace_xy(std::span<location const> c, displacement_profile dy);
}

#endif
