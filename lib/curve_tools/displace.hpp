//@	{"dependencies_extra":[{"ref":"./displace.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_CURVE_DISPLACE_HPP
#define TERRAFORMER_CURVE_DISPLACE_HPP

#include "lib/common/spaces.hpp"
#include "lib/array_classes/multi_array.hpp"
#include "lib/curve_tools/length.hpp"

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

	class displaced_curve : public multi_array<location, float>
	{
	public:
		using multi_array<location, float>::multi_array;

		decltype(auto) scalar_displacements() const
		{ return get<1>(); }

		decltype(auto) scalar_displacements()
		{ return get<1>(); }

		decltype(auto) points() const
		{ return get<0>(); }

		decltype(auto) points()
		{ return get<0>(); }
	};

	displaced_curve displace_xy(std::span<location const> c, displacement_profile dy);

	inline void replace_z_inplace(span<location> c, span<float const> z_vals)
	{
		for(auto k = c.first_element_index() ; k != std::size(c); ++k)
		{
			array_index<float> src_index{k.get()};
			c[k][2] = z_vals[src_index];
		}
	}
}

#endif
