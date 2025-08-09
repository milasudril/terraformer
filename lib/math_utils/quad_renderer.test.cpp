//@	{"target":{"name": "quad_renderer.test"}}

#include "./quad_renderer.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_quad_renderer_map_unit_square_to_quad)
{
	terraformer::quad const q{
		.p1 = terraformer::location{-1.0f, -4.0f, 0.0f},
		.p2 = terraformer::location{ 3.0f,  2.0f, 0.0f},
		.p3 = terraformer::location{-4.0f,  1.0f, 0.0f},
		.p4 = terraformer::location{-2.0f,  2.0f, 0.0f}
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