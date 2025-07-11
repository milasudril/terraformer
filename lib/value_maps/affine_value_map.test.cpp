//@	{"target":{"name": "affine_value_map.test"}}

#include "./affine_value_map.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_value_maps_affine_value_map_verify)
{
	terraformer::value_maps::affine_value_map map{-2.0f, 6.0f};
	EXPECT_EQ(map.min(), -2.0f);
	EXPECT_EQ(map.max(), 6.0f);
	EXPECT_EQ(map.from_value(2.0f), 0.5f);
	EXPECT_EQ(map.to_value(0.5f), 2.0f);
	EXPECT_EQ(map.from_value(6.0f), 1.0f);
	EXPECT_EQ(map.to_value(1.0f), 6.0f);
	EXPECT_EQ(map.from_value(-2.0f), 0.0f);
	EXPECT_EQ(map.to_value(0.0f), -2.0f);
}