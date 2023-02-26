//@	{"target":{"name":"coordinate_transforms.test"}}

#include "./coordinate_transforms.hpp"

#include "testfwk/testfwk.hpp"

/*

	inline auto to_colatitude(geosimd::turn_angle latitude)
	{
		return geosimd::rotation_angle{0x4000'0000} - latitude;
	}

	struct longcolat
	{
		geosimd::rotation_angle longitude;
		geosimd::rotation_angle colatitude;
	};

	inline auto to_longcolat(hires_location loc,
		double planet_radius,
		geosimd::rotation_angle colat_offset)
	{
		auto const theta = colat_offset
			+ geosimd::turn_angle{
				geosimd::rad{loc[1]/planet_radius}
			};
		geosimd::rotation_angle const phi{
			geosimd::rad{loc[0]/(planet_radius*sin(theta))}
		};

		return longcolat{
			.longitude = phi,
			.colatitude = theta
		};
	}*/

TESTCASE(terraformer_to_map_location_origin)
{
	auto const loc = to_map_location(terraformer::pixel_coordinates{
			.x = 0,
			.y = 0,
		},
		terraformer::span_2d_extents{
			.width = 63,
			.height = 53
		},
		3.0);

	EXPECT_EQ(loc[0], (-0.5*63 + 0.5)*3.0);
	EXPECT_EQ(loc[1], (-0.5*53 + 0.5)*3.0);
}

TESTCASE(terraformer_to_map_location_end)
{
	auto const loc = to_map_location(terraformer::pixel_coordinates{
			.x = 62,
			.y = 52,
		},
		terraformer::span_2d_extents{
			.width = 63,
			.height = 53
		},
		3.0);

	EXPECT_EQ(loc[0], (-0.5*63 + 0.5 + 62)*3.0);
	EXPECT_EQ(loc[1], (-0.5*53 + 0.5 + 52)*3.0);
}

TESTCASE(terraformer_to_map_location_mid)
{
	auto const loc = to_map_location(terraformer::pixel_coordinates{
			.x = 31,
			.y = 26,
		},
		terraformer::span_2d_extents{
			.width = 63,
			.height = 53
		},
		3.0);

	EXPECT_EQ(loc[0], 0.0);
	EXPECT_EQ(loc[1], 0.0);
}

TESTCASE(terraformer_to_map_location_origin_different_pixel_size)
{
	auto const loc = to_map_location(terraformer::pixel_coordinates{
			.x = 0,
			.y = 0,
		},
		terraformer::span_2d_extents{
			.width = 63,
			.height = 53
		},
		4.0);

	EXPECT_EQ(loc[0], (-0.5*63 + 0.5)*4.0);
	EXPECT_EQ(loc[1], (-0.5*53 + 0.5)*4.0);
}

TESTCASE(terraformer_to_colatitude)
{
	auto const a = terraformer::to_colatitude(geosimd::turn_angle{-0x2000'0000});
	auto const b = terraformer::to_colatitude(geosimd::turn_angle{0x2000'0000});

	EXPECT_EQ(a.get(), geosimd::rotation_angle{0x6000'0000}.get());
	EXPECT_EQ(b.get(), geosimd::rotation_angle{0x2000'0000}.get());
}

TESTCASE(terraformer_to_longcolat_origin_equator)
{
	auto const res = terraformer::to_longcolat(terraformer::hires_origin,
		2.0,
		terraformer::to_colatitude(geosimd::turn_angle{0x0}));

	EXPECT_EQ(res.longitude.get(), geosimd::rotation_angle{0x0}.get());
	EXPECT_EQ(res.colatitude.get(), geosimd::rotation_angle{0x4000'0000}.get());
}


TESTCASE(terraformer_to_longcolat_origin_45N)
{
	auto const res = terraformer::to_longcolat(terraformer::hires_origin,
		2.0,
		terraformer::to_colatitude(geosimd::turn_angle{0x2000'0000}));

	EXPECT_EQ(res.longitude.get(), geosimd::rotation_angle{0x0}.get());
	EXPECT_EQ(res.colatitude.get(), geosimd::rotation_angle{0x2000'0000}.get());
}

TESTCASE(terraformer_to_longcolat_nw_equator)
{
	auto const res = terraformer::to_longcolat(terraformer::hires_location{-1.5, -1.0, 0.0},
		4.0,
		terraformer::to_colatitude(geosimd::turn_angle{0x0}));

	EXPECT_EQ(res.longitude.get(), geosimd::rotation_angle{0xF03B'1C6C}.get());
	EXPECT_EQ(res.colatitude.get(), geosimd::rotation_angle{0x35D0'67CA}.get());
}

TESTCASE(terraformer_to_longcolat_w_equator)
{
	auto const res = terraformer::to_longcolat(terraformer::hires_location{-1.5, 0.0, 0.0},
		4.0,
		terraformer::to_colatitude(geosimd::turn_angle{0x0}));

	EXPECT_EQ(res.longitude.get(), geosimd::rotation_angle{0xF0B8'9BAE}.get());
	EXPECT_EQ(res.colatitude.get(), geosimd::rotation_angle{0x4000'0000}.get());
}

TESTCASE(terraformer_to_longcolat_ne_equator_other_radius)
{
	auto const res = terraformer::to_longcolat(terraformer::hires_location{1.5, -1.0, 0.0},
		8.0,
		terraformer::to_colatitude(geosimd::turn_angle{0x0}));

	EXPECT_EQ(res.longitude.get(), geosimd::rotation_angle{0x7B3'132A}.get());
	EXPECT_EQ(res.colatitude.get(), geosimd::rotation_angle{0x3AE8'33E5}.get());
}

TESTCASE(terraformer_planet_location)
{
	auto const loc_a1 = planet_location(terraformer::year{0.0}, 1.0);
	auto const loc_b1 = planet_location(terraformer::year{0.25}, 1.0);
	auto const loc_c1 = planet_location(terraformer::year{0.5}, 1.0);
	auto const loc_d1 = planet_location(terraformer::year{0.75}, 1.0);

	EXPECT_EQ(loc_a1[0], 1.0);
	EXPECT_EQ(loc_a1[1], 0.0);
	EXPECT_EQ(loc_b1[0], 0.0);
	EXPECT_EQ(loc_b1[1], 1.0);
	EXPECT_EQ(loc_c1[0], -1.0);
	EXPECT_EQ(loc_c1[1], 0.0);
	EXPECT_EQ(loc_d1[0], 0.0);
	EXPECT_EQ(loc_d1[1], -1.0);

	auto const loc_a2 = planet_location(terraformer::year{0.0}, 2.0);
	auto const loc_b2 = planet_location(terraformer::year{0.25}, 2.0);
	auto const loc_c2 = planet_location(terraformer::year{0.5}, 2.0);
	auto const loc_d2 = planet_location(terraformer::year{0.75}, 2.0);

	EXPECT_EQ(loc_a2[0], 2.0);
	EXPECT_EQ(loc_a2[1], 0.0);
	EXPECT_EQ(loc_b2[0], 0.0);
	EXPECT_EQ(loc_b2[1], 2.0);
	EXPECT_EQ(loc_c2[0], -2.0);
	EXPECT_EQ(loc_c2[1], 0.0);
	EXPECT_EQ(loc_d2[0], 0.0);
	EXPECT_EQ(loc_d2[1], -2.0);
}

TESTCASE(terraformer_planet_rotation_angles)
{
	auto const rot = terraformer::planet_rotation(geosimd::turn_angle{0x1000'0000},
		geosimd::turn_angle{0x2000'0000});

	auto const v = terraformer::hires_displacement{1.0, 0.0, 0.0}.apply(rot);
	EXPECT_LT(std::abs(v[0] - 0.65328148f), 1e-7);
	EXPECT_LT(std::abs(v[1] - 0.38268343f), 1e-7);
	EXPECT_LT(std::abs(v[2] + 0.65328148f), 1e-7);
}

TESTCASE(terraformer_planet_rotation_time)
{
	terraformer::year saved_time{0.0};
	auto const rot = planet_rotation(terraformer::year{0.125}, 0.5, [&saved_time](auto t){
		saved_time = t;
		return geosimd::turn_angle{0x2000'0000};
	});

	EXPECT_EQ(saved_time.value(), 0.125);

	auto const v = terraformer::hires_displacement{1.0, 0.0, 0.0}.apply(rot);
	EXPECT_LT(std::abs(v[0] - 0.65328148f), 1e-7);
	EXPECT_LT(std::abs(v[1] - 0.38268343f), 1e-7);
	EXPECT_LT(std::abs(v[2] + 0.65328148f), 1e-7);
}