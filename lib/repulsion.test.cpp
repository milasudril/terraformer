//@	{"target":{"name":"repulsion.test"}}

#include "./repulsion.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_repulsion_loc_points_no_points)
{
	auto x = terraformer::repulsion_between(terraformer::location{},
		std::span<terraformer::location const>{});

	EXPECT_EQ(x, terraformer::displacement{});
}

TESTCASE(terraformer_repulsion_loc_points_one_points)
{
	terraformer::location loc{};

	auto x = terraformer::repulsion_between(terraformer::location{},
		std::span<terraformer::location const>{&loc, 1});

	EXPECT_EQ(x, terraformer::displacement{});
}

TESTCASE(terraformer_repulsion_loc_points_loc_north_of_we_line_segment)
{
	std::array<terraformer::location, 2> segs{
		terraformer::location{-1.0f, 0.0f, 0.0f},
		terraformer::location{ 1.0f, 0.0f, 0.0f},
	};

	for(size_t k = 0; k != 4; ++k)
	{
		auto const v = terraformer::displacement{0.0f, static_cast<float>(k + 1), 0.0f};
		auto const x = terraformer::repulsion_between(terraformer::origin + v, segs);
 		auto const expected_field_strength = 2.0f/norm_squared(v);
		EXPECT_EQ(x, expected_field_strength*terraformer::direction{terraformer::geom_space::y{}});
	}
}

TESTCASE(terraformer_repulsion_loc_points_loc_south_of_we_line_segment)
{
	std::array<terraformer::location, 2> segs{
		terraformer::location{-1.0f, 0.0f, 0.0f},
		terraformer::location{ 1.0f, 0.0f, 0.0f},
	};

	for(size_t k = 0; k != 4; ++k)
	{
		auto const v = terraformer::displacement{0.0f, static_cast<float>(k + 1), 0.0f};
		auto const x = terraformer::repulsion_between(terraformer::origin - v, segs);
 		auto const expected_field_strength = 2.0f/norm_squared(v);
		EXPECT_EQ(x, -expected_field_strength*terraformer::direction{terraformer::geom_space::y{}});
	}
}

TESTCASE(terraformer_repulsion_loc_points_loc_east_of_sn_line_segment)
{
	std::array<terraformer::location, 2> segs{
		terraformer::location{0.0f, -1.0f, 0.0f},
		terraformer::location{0.0f, 1.0f, 0.0f},
	};

	for(size_t k = 0; k != 4; ++k)
	{
		auto const v = terraformer::displacement{static_cast<float>(k + 1), 0.0f, 0.0f};
		auto const x = terraformer::repulsion_between(terraformer::origin + v, segs);
 		auto const expected_field_strength = 2.0f/norm_squared(v);
		EXPECT_EQ(x, expected_field_strength*terraformer::direction{terraformer::geom_space::x{}});
	}
}

TESTCASE(terraformer_repulsion_loc_points_loc_west_of_sn_line_segment)
{
	std::array<terraformer::location, 2> segs{
		terraformer::location{0.0f, -1.0f, 0.0f},
		terraformer::location{0.0f, 1.0f, 0.0f},
	};

	for(size_t k = 0; k != 4; ++k)
	{
		auto const v = terraformer::displacement{static_cast<float>(k + 1), 0.0f, 0.0f};
		auto const x = terraformer::repulsion_between(terraformer::origin - v, segs);
 		auto const expected_field_strength = 2.0f/norm_squared(v);
		EXPECT_EQ(x, -expected_field_strength*terraformer::direction{terraformer::geom_space::x{}});
	}
}

TESTCASE(terraformer_repulsion_loc_points_loc_inside_closed_curve)
{
	std::array<terraformer::location, 97> segs;
	for(size_t k = 0; k != std::size(segs); ++k)
	{
		geosimd::turn_angle theta{geosimd::turns{static_cast<double>(k)/(std::size(segs) - 1)}};
		auto const cs_3theta = cossin(3*theta);
		auto const cs_theta = cossin(theta);
		auto const r = 2.0f + cs_3theta.sin;
		segs[k] = terraformer::origin
			+ r*terraformer::displacement{cs_theta.cos, cs_theta.sin, 0.0f};
	}

	auto const x = terraformer::repulsion_between(terraformer::origin, segs);
	EXPECT_LT(norm(x), 1.0f/1048576.0f);
}