//@	{"target":{"name":"sun_direction.test"}}

#include "./sun_direction.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_geomodels_sun_direction)
{
	terraformer::hires_location const earth_loc{-8.0, 0.0, 0.0};
	terraformer::hires_displacement const view_offset_a{5.0, 0.0, 0.0};
	terraformer::hires_displacement const view_offset_b{4.0, 0.0, 3.0};
	terraformer::hires_displacement const view_offset_c{0.0, 4.0, 3.0};

	printf("A\n");

	auto const orbit_dir_a = terraformer::orbital_sun_direction(earth_loc, view_offset_a);
	printf("%.8g %.8g %.8g\n", orbit_dir_a[0], orbit_dir_a[1], orbit_dir_a[2]);
	auto const local_dir_a = terraformer::local_sun_direction(earth_loc, view_offset_a);
	printf("%.8g %.8g %.8g\n", local_dir_a[0], local_dir_a[1], local_dir_a[2]);

	printf("B\n");
	auto const orbit_dir_b = terraformer::orbital_sun_direction(earth_loc, view_offset_b);
	printf("%.8g %.8g %.8g\n", orbit_dir_b[0], orbit_dir_b[1], orbit_dir_b[2]);
	auto const local_dir_b = terraformer::local_sun_direction(earth_loc, view_offset_b);
	printf("%.8g %.8g %.8g\n", local_dir_b[0], local_dir_b[1], local_dir_b[2]);

	printf("C\n");
	auto const orbit_dir_c= terraformer::orbital_sun_direction(earth_loc, view_offset_c);
	printf("%.8g %.8g %.8g\n", orbit_dir_c[0], orbit_dir_c[1], orbit_dir_c[2]);
	auto const local_dir_c = terraformer::local_sun_direction(earth_loc, view_offset_c);
	printf("%.8g %.8g %.8g\n", local_dir_c[0], local_dir_c[1], local_dir_c[2]);
}