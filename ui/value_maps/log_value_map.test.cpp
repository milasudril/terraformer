//@	{"target":{"name": "log_value_map.test"}}

#include "./log_value_map.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_value_maps_log_value_map_verify_base_2)
{
	terraformer::ui::value_maps::log_value_map map{1.0f/256.0f, 256.0f, 2.0f};
	EXPECT_EQ(map.min(), 1.0f/256.0f);
	EXPECT_EQ(map.max(), 256.0f);
	EXPECT_EQ(map.from_value(1.0f), 0.5f);
	EXPECT_EQ(map.to_value(0.5f), 1.0f);
	EXPECT_EQ(map.from_value(256.0f), 1.0f);
	EXPECT_EQ(map.to_value(1.0f), 256.0f);
	EXPECT_EQ(map.from_value(1.0f/256.0f), 0.0f);
	EXPECT_EQ(map.to_value(0.0f), 1.0f/256.0f);
	EXPECT_EQ(map.from_value(0.0f), 0.0f);
	EXPECT_EQ(map.from_value(-1.0f), 0.0f);
}

TESTCASE(terraformer_ui_value_maps_log_value_map_verify_base_4)
{
	terraformer::ui::value_maps::log_value_map map{1.0f/256.0f, 256.0f, 4.0f};
	EXPECT_EQ(map.min(), 1.0f/256.0f);
	EXPECT_EQ(map.max(), 256.0f);
	EXPECT_EQ(map.from_value(1.0f), 0.5f);
	EXPECT_EQ(map.to_value(0.5f), 1.0f);
	EXPECT_EQ(map.from_value(256.0f), 1.0f);
	EXPECT_EQ(map.to_value(1.0f), 256.0f);
	EXPECT_EQ(map.from_value(1.0f/256.0f), 0.0f);
	EXPECT_EQ(map.to_value(0.0f), 1.0f/256.0f);
	EXPECT_EQ(map.from_value(0.0f), 0.0f);
	EXPECT_EQ(map.from_value(-1.0f), 0.0f);
}