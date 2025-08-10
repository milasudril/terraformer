//@	{"target":{"name": "quad_renderer.test"}}

#include "./quad_renderer.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_quad_renderer_map_unit_square_to_quad)
{
	terraformer::quad const q{
		.p1 = terraformer::location{-1.0f, -4.0f, 0.0f},
		.p2 = terraformer::location{ 3.0f,  2.0f, 0.0f},
		.p3 = terraformer::location{-2.0f,  2.0f, 0.0f},
		.p4 = terraformer::location{-4.0f,  1.0f, 0.0f}
	};

	auto const loc1 = terraformer::map_unit_square_to_quad(terraformer::location{}, q);
	EXPECT_EQ(loc1, q.p1);


	auto const loc2 = terraformer::map_unit_square_to_quad(terraformer::location{1.0f, 0.0f, 0.0f}, q);
	EXPECT_EQ(loc2, q.p2);

	auto const loc3 = terraformer::map_unit_square_to_quad(terraformer::location{0.0f, 1.0f, 0.0f}, q);
	EXPECT_EQ(loc3, q.p3);

	auto const loc4 = terraformer::map_unit_square_to_quad(terraformer::location{1.0f, 1.0f, 0.0f}, q);
	EXPECT_EQ(loc4, q.p4);
}

TESTCASE(terraformer_quad_renderer_map_quad_to_unit_square)
{
	terraformer::quad const q{
		.p1 = terraformer::location{-1.0f, -4.0f, 0.0f},
		.p2 = terraformer::location{ 3.0f,  2.0f, 0.0f},
		.p3 = terraformer::location{-2.0f,  2.0f, 0.0f},
		.p4 = terraformer::location{-4.0f,  1.0f, 0.0f}
	};

	auto const loc1 = terraformer::map_quad_to_unit_square(q, q.p1);
	EXPECT_EQ(loc1, terraformer::location{});

	auto const loc2 = terraformer::map_quad_to_unit_square(q, q.p2);
	EXPECT_EQ(loc2, (terraformer::location{1.0f, 0.0f, 0.0f}));

	auto const loc3 = terraformer::map_quad_to_unit_square(q, q.p3);
	EXPECT_EQ(loc3, (terraformer::location{0.0f, 1.0f, 0.0f}));

	auto const loc4 = terraformer::map_quad_to_unit_square(q, q.p4);
	EXPECT_LT(distance(loc4, terraformer::location{1.0f, 1.0f, 0.0f}), 1.0e-6f);

	auto const loc_outside1 = terraformer::map_quad_to_unit_square(
		q,
		terraformer::location{-3.0f, -2.0f, 0.0f}
	);
	EXPECT_EQ(
			(loc_outside1[0] >= 0.0f && loc_outside1[0] <= 1.0f)
		&& (loc_outside1[1] >= 0.0f && loc_outside1[1] <= 1.0f),
		false
	);

	auto const loc_outside2 = terraformer::map_quad_to_unit_square(
		q,
		terraformer::location{2.0f, -2.0f, 0.0f}
	);
	EXPECT_EQ(
			(loc_outside2[0] >= 0.0f && loc_outside2[0] <= 1.0f)
		&& (loc_outside2[1] >= 0.0f && loc_outside2[1] <= 1.0f),
		false
	);

	auto const loc_outside3 = terraformer::map_quad_to_unit_square(
		q,
		terraformer::location{0.0f, 3.0f, 0.0f}
	);
	EXPECT_EQ(
			(loc_outside3[0] >= 0.0f && loc_outside3[0] <= 1.0f)
		&& (loc_outside3[1] >= 0.0f && loc_outside3[1] <= 1.0f),
		false
	);

	auto const loc_outside4 = terraformer::map_quad_to_unit_square(
		q,
		terraformer::location{-4.0f, 2.0f, 0.0f}
	);
	EXPECT_EQ(
			(loc_outside4[0] >= 0.0f && loc_outside4[0] <= 1.0f)
		&& (loc_outside4[1] >= 0.0f && loc_outside4[1] <= 1.0f),
		false
	);

	auto const loc_inside = terraformer::map_quad_to_unit_square(
		q,
		terraformer::location{}
	);
	EXPECT_EQ(
			(loc_inside[0] > 0.0f && loc_inside[0] < 1.0f)
		&& (loc_inside[1] > 0.0f && loc_inside[1] < 1.0f),
		true
	);
}

TESTCASE(terraformer_quad_renderer_map_quad_to_unit_square_crossed)
{
	terraformer::quad const q{
		.p1 = terraformer::location{-1.0f, -4.0f, 0.0f},
		.p2 = terraformer::location{ 3.0f,  2.0f, 0.0f},
		.p3 = terraformer::location{-4.0f,  1.0f, 0.0f},
		.p4 = terraformer::location{-2.0f,  2.0f, 0.0f}
	};

	auto const loc1 = terraformer::map_quad_to_unit_square(q, q.p1);
	EXPECT_EQ(loc1, terraformer::location{});

	auto const loc2 = terraformer::map_quad_to_unit_square(q, q.p2);
	EXPECT_EQ(loc2, (terraformer::location{1.0f, 0.0f, 0.0f}));

	auto const loc3 = terraformer::map_quad_to_unit_square(q, q.p3);
	EXPECT_EQ(loc3, (terraformer::location{0.0f, 1.0f, 0.0f}));

	auto const loc4 = terraformer::map_quad_to_unit_square(q, q.p4);
	EXPECT_LT(distance(loc4, terraformer::location{1.0f, 1.0f, 0.0f}), 1.0e-6f);
}

TESTCASE(terraformer_quad_renderer_map_quad_to_unit_square_degenerate)
{
	terraformer::quad const q{
		.p1 = terraformer::location{-1.0f, -4.0f, 0.0f},
		.p2 = terraformer::location{ 3.0f,  2.0f, 0.0f},
		.p3 = terraformer::location{-2.0f,  2.0f, 0.0f},
		.p4 = terraformer::location{-2.0f,  2.0f, 0.0f}
	};

	auto const loc1 = terraformer::map_quad_to_unit_square(q, q.p1);
	EXPECT_EQ(loc1, terraformer::location{});

	auto const loc2 = terraformer::map_quad_to_unit_square(q, q.p2);
	EXPECT_EQ(loc2, (terraformer::location{1.0f, 0.0f, 0.0f}));

	// 3 and 4 are indistinguishable
}


