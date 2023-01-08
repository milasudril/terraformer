#ifndef TERRAFORMER_LIB_SPACES_HPP
#define TERRAFORMER_LIB_SPACES_HPP

#include <geosimd/euclidian_space.hpp>

namespace terraformer
{
	struct geom_space : geosimd::euclidian_space<float, 3>{};

	using location = geom_space::location;
	using displacement = geom_space::displacement;
	using direction = geom_space::direction;

	inline constexpr auto origin = geosimd::origin<geosimd::euclidian_space<float, 3>>();
};

#endif