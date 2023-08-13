#ifndef TERRAFORMER_LIB_SPACES_HPP
#define TERRAFORMER_LIB_SPACES_HPP

#include <geosimd/euclidian_space.hpp>
#include <geosimd/rotation.hpp>
#include <geosimd/scaling.hpp>

namespace terraformer
{
	using geom_space = geosimd::euclidian_space<float, 3>;
	using location = geom_space::location;
	static_assert(geosimd::vector_space<geosimd::euclidian_space<float, 3>>);
	using displacement = geom_space::displacement;
	using direction = geom_space::direction;
	using rotation = geosimd::rotation<geom_space>;
	using scaling = geosimd::scaling<geom_space>;
	inline constexpr auto origin = geosimd::origin<geosimd::euclidian_space<float, 3>>();

	using hires_geom_space = geosimd::euclidian_space<double, 3>;
	using hires_location = hires_geom_space::location;
	using hires_displacement = hires_geom_space::displacement;
	using hires_direction = hires_geom_space::direction;
	using hires_rotation = geosimd::rotation<hires_geom_space>;
	inline constexpr auto hires_origin = geosimd::origin<geosimd::euclidian_space<double, 3>>();

	template<class Point>
	inline constexpr auto distance_xy(Point a, Point b)
	{
		auto dr = a - b;
		auto val = dr.get()*dr.get();
		return std::sqrt(val[0] + val[1]);
	}
};


#endif
