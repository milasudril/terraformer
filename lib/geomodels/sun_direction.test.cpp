//@	{"target":{"name":"sun_direction.test"}}

#include "./sun_direction.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_N45E0_00UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0x8000'0000},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x0000'0000},
		geosimd::rotation_angle{0x2000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), 0.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), -1.0f/std::sqrt(2.0f));
	EXPECT_EQ(static_cast<float>(local_dir_a[2]), -1.0f/std::sqrt(2.0f));
}

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_N45E0_06UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0xc000'0000},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x0000'0000},
		geosimd::rotation_angle{0x2000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), 1.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), 0.0f);
	EXPECT_LT(std::abs(static_cast<float>(local_dir_a[2])), 1.0e-7f);
}

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_N45E0_12UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0x0},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x0000'0000},
		geosimd::rotation_angle{0x2000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), 0.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), 1.0f/std::sqrt(2.0f));
	EXPECT_EQ(static_cast<float>(local_dir_a[2]), 1.0f/std::sqrt(2.0f));
}

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_N45E0_18UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0x4000'0000},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x0000'0000},
		geosimd::rotation_angle{0x2000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), -1.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), 0.0f);
	EXPECT_LT(std::abs(static_cast<float>(local_dir_a[2])), 1.0e-7f);
}

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_N45E90_00UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0x8000'0000},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x4000'0000},
		geosimd::rotation_angle{0x2000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), 1.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), 0.0f);
	EXPECT_LT(std::abs(static_cast<float>(local_dir_a[2])), 1.0e-7f);
}

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_N45E90_06UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0xc000'0000},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x4000'0000},
		geosimd::rotation_angle{0x2000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), 0.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), 1.0f/std::sqrt(2.0f));
	EXPECT_EQ(static_cast<float>(local_dir_a[2]), 1.0f/std::sqrt(2.0f));
}

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_N45E90_12UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0x0},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x4000'0000},
		geosimd::rotation_angle{0x2000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), -1.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), 0.0f);
	EXPECT_LT(std::abs(static_cast<float>(local_dir_a[2])), 1.0e-7f);
}

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_N45E90_18UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0x4000'0000},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x4000'0000},
		geosimd::rotation_angle{0x2000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), 0.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), -1.0f/std::sqrt(2.0f));
	EXPECT_EQ(static_cast<float>(local_dir_a[2]), -1.0f/std::sqrt(2.0f));
}

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_S45E0_00UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0x8000'0000},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x0000'0000},
		geosimd::rotation_angle{0x6000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), 0.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), 1.0f/std::sqrt(2.0f));
	EXPECT_EQ(static_cast<float>(local_dir_a[2]), -1.0f/std::sqrt(2.0f));
}

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_S45E0_06UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0xc000'0000},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x0000'0000},
		geosimd::rotation_angle{0x6000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), 1.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), 0.0f);
	EXPECT_LT(std::abs(static_cast<float>(local_dir_a[2])), 1.0e-7f);
}

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_S45E0_12UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0x0},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x0000'0000},
		geosimd::rotation_angle{0x6000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), 0.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), -1.0f/std::sqrt(2.0f));
	EXPECT_EQ(static_cast<float>(local_dir_a[2]), 1.0f/std::sqrt(2.0f));
}

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_S45E0_18UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0x4000'0000},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x0000'0000},
		geosimd::rotation_angle{0x6000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), -1.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), 0.0f);
	EXPECT_LT(std::abs(static_cast<float>(local_dir_a[2])), 1.0e-7f);
}

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_S45E90_00UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0x8000'0000},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x4000'0000},
		geosimd::rotation_angle{0x6000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), 1.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), 0.0f);
	EXPECT_LT(std::abs(static_cast<float>(local_dir_a[2])), 1.0e-7f);
}

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_S45E90_06UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0xc000'0000},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x4000'0000},
		geosimd::rotation_angle{0x6000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), 0.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), -1.0f/std::sqrt(2.0f));
	EXPECT_EQ(static_cast<float>(local_dir_a[2]), 1.0f/std::sqrt(2.0f));
}

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_S45E90_12UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0x0},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x4000'0000},
		geosimd::rotation_angle{0x6000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), -1.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), 0.0f);
	EXPECT_LT(std::abs(static_cast<float>(local_dir_a[2])), 1.0e-7f);
}

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_S45E90_18UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0x4000'0000},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x4000'0000},
		geosimd::rotation_angle{0x6000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), 0.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), 1.0f/std::sqrt(2.0f));
	EXPECT_EQ(static_cast<float>(local_dir_a[2]), -1.0f/std::sqrt(2.0f));
}

TESTCASE(terraformer_geomodels_local_sun_direction_no_tilt_N90_00UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> const rot_a{
		geosimd::rotation_angle{0x0000'0000},
		geosimd::dimension_tag<2>{}
	};
	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x0},
		geosimd::rotation_angle{0x0}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), 0.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), 1.0f);
	EXPECT_LT(std::abs(static_cast<float>(local_dir_a[2])), 1e-7);
}

TESTCASE(terraformer_geomodels_local_sun_direction_T22_N67E0_00UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> rot_a{};

	rot_a
		.push(geosimd::turn_angle{0x8000'0000}, geosimd::dimension_tag<2>{})
		.push(geosimd::turn_angle{0x1000'0000}, geosimd::dimension_tag<1>{});

	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x0},
		geosimd::rotation_angle{0x1000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), 0.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), -1.0f/std::sqrt(2.0f));
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), -1.0f/std::sqrt(2.0f));
}

TESTCASE(terraformer_geomodels_local_sun_direction_T22_N67E0_06UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> rot_a{};

	rot_a
		.push(geosimd::turn_angle{0xc000'0000}, geosimd::dimension_tag<2>{})
		.push(geosimd::turn_angle{0x1000'0000}, geosimd::dimension_tag<1>{});

	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x0},
		geosimd::rotation_angle{0x1000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), 1.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), 0.0f);
	EXPECT_LT(std::abs(static_cast<float>(local_dir_a[2])), 1.0e-7f);
}

TESTCASE(terraformer_geomodels_local_sun_direction_T22_N67E0_12UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> rot_a{};

	rot_a
		.push(geosimd::turn_angle{0x0000'0000}, geosimd::dimension_tag<2>{})
		.push(geosimd::turn_angle{0x1000'0000}, geosimd::dimension_tag<1>{});

	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x0},
		geosimd::rotation_angle{0x1000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), 0.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), 1.0f/std::sqrt(2.0f));
	EXPECT_EQ(static_cast<float>(local_dir_a[2]), 1.0f/std::sqrt(2.0f));
}

TESTCASE(terraformer_geomodels_local_sun_direction_T22_N67E0_18UTC)
{
	terraformer::hires_location const planet_loc{-1024.0*1024.0*1024.0, 0.0, 0.0};

	geosimd::rotation<terraformer::hires_geom_space> rot_a{};

	rot_a
		.push(geosimd::turn_angle{0x4000'0000}, geosimd::dimension_tag<2>{})
		.push(geosimd::turn_angle{0x1000'0000}, geosimd::dimension_tag<1>{});

	auto const local_dir_a = terraformer::local_sun_direction(planet_loc,
		rot_a,
		geosimd::rotation_angle{0x0},
		geosimd::rotation_angle{0x1000'0000}
	);

	EXPECT_EQ(static_cast<float>(local_dir_a[0]), -1.0f);
	EXPECT_EQ(static_cast<float>(local_dir_a[1]), 0.0f);
	EXPECT_LT(std::abs(static_cast<float>(local_dir_a[2])), 1.0e-7f);
}