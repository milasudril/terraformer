//@	{"target":{"name":"sun_direction.test"}}

#include "./sun_direction.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_geomodels_local_sun_direction)
{
/*
 hires_location planet_location,
		geosimd::rotation<hires_geom_space> const& planet_rotation,
		geosimd::rotation_angle longitude,
		geosimd::rotation_angle colatitude*/

	terraformer::hires_location const planet_loc{-1024.0*1024.0, 0.0, 0.0};
	geosimd::rotation<terraformer::hires_geom_space> const rot{geosimd::rotation_angle{0x0}, geosimd::dimension_tag<2>{}};


	printf("A\n");

	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot,
		geosimd::rotation_angle{0x0000'0000},
		geosimd::rotation_angle{0x2000'0000}
	);
	printf("%.8g %.8g %.8g\n", local_dir_a[0], local_dir_a[1], local_dir_a[2]);
#if 0
	printf("B\n");
	auto const local_dir_b = terraformer::local_sun_direction(earth_loc, view_offset_b);
	printf("%.8g %.8g %.8g\n", local_dir_b[0], local_dir_b[1], local_dir_b[2]);

	printf("C\n");
	auto const local_dir_c = terraformer::local_sun_direction(earth_loc, view_offset_c);
	printf("%.8g %.8g %.8g\n", local_dir_c[0], local_dir_c[1], local_dir_c[2]);
#endif
}