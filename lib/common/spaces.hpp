#ifndef TERRAFORMER_LIB_SPACES_HPP
#define TERRAFORMER_LIB_SPACES_HPP

#include <geosimd/euclidian_space.hpp>

namespace terraformer
{
	using geom_space = geosimd::euclidian_space<float, 3>;

	using location = geom_space::location;
	using displacement = geom_space::displacement;
	using direction = geom_space::direction;

	inline constexpr auto origin = geosimd::origin<geosimd::euclidian_space<float, 3>>();


	using hires_geom_space = geosimd::euclidian_space<double, 3>;
	using hires_location = hires_geom_space::location;
	using hires_displacement = hires_geom_space::displacement;
	using hires_direction = hires_geom_space::direction;

	inline constexpr auto hires_origin = geosimd::origin<geosimd::euclidian_space<double, 3>>();
};

#endif
