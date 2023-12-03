//@	{"dependencies_extra":[{"ref":"./curve_displace.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_CURVE_DISPLACE_HPP
#define TERRAFORMER_CURVE_DISPLACE_HPP

#include "lib/common/spaces.hpp"

#include <span>

namespace terraformer
{
	inline direction curve_vertex_normal(location a, location b, location c)
	{
		displacement const tangent = c - a;
		auto const binormal = cross(tangent, b - a);
		return direction{cross(binormal, tangent)};
	}

	void displace(std::span<location> c, std::span<displacement const> dx);
}

#endif