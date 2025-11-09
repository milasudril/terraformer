//@	{"dependencies_extra":[{"ref":"./displace.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_CURVE_DISPLACE_HPP
#define TERRAFORMER_CURVE_DISPLACE_HPP

#include "lib/common/spaces.hpp"
#include "lib/array_classes/multi_array.hpp"
#include "lib/curve_tools/length.hpp"
#include "lib/math_utils/fp_props.hpp"

#include <span>
#include <bit>

namespace terraformer
{
	inline std::optional<direction> curve_vertex_normal_from_curvature(
		location a,
		location b,
		location c)
	{
		auto const tangent = c - a;
		auto const binormal = cross(tangent, b - a);
		auto const normal = cross(binormal, tangent);
		auto const l = norm(normal);
		if(l == 0.0f)
		{ return std::nullopt; }
		return direction{normal/l, direction::prenormalized_tag{}};
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

	class displaced_curve : public multi_array<location, float, location>
	{
	public:
		using multi_array<location, float, location>::multi_array;

		decltype(auto) scalar_displacements() const
		{ return get<1>(); }

		decltype(auto) scalar_displacements()
		{ return get<1>(); }

		decltype(auto) points() const
		{ return get<0>(); }

		decltype(auto) points()
		{ return get<0>(); }

		decltype(auto) input_points() const
		{ return get<2>(); }

		decltype(auto) input_points()
		{ return get<2>(); }
	};

	displaced_curve displace_xy(std::span<location const> c, displacement_profile dy);

	inline void replace_z_inplace(span<location> c, span<float const> z_vals)
	{
		for(auto k : c.element_indices())
		{
			array_index<float> src_index{k.get()};
			c[k][2] = z_vals[src_index];
		}
	}
}

#endif
