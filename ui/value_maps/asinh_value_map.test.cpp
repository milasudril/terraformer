//@	{"target":{"name": "asinh_value_map.test"}}

#include "./asinh_value_map.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_value_maps_asinh_value_map_verify)
{
	terraformer::ui::value_maps::asinh_value_map map{16.0f, 64.0f};
	EXPECT_LT(std::abs(map.min() - (-64.0f)), 1.0e-5f);
	EXPECT_LT(std::abs(map.max() - 64.0f), 1.0e-5f);
	EXPECT_EQ(map.from_value(0.0f), 0.5f);
	EXPECT_EQ(map.to_value(0.5f), 0.0f);
	EXPECT_EQ(map.from_value(64.0f), 1.0f);
	EXPECT_LT(std::abs(map.to_value(1.0f) - 64.0f), 1.0e-5f);
	EXPECT_EQ(map.from_value(-64.0f), 0.0f);
	EXPECT_LT(std::abs(map.to_value(0.0f) - (-64.0f)), 1.0e-5f);

	EXPECT_LT(std::abs(map.to_value(map.from_value(16.0f)) - 16.0f), 1.0e-5f);
	EXPECT_LT(std::abs(map.from_value(map.to_value(0.75f)) - 0.75f), 1.0e-5f);

	auto const factor = std::asinh(64.0f*std::sinh(1.0f)/16.0f);
	EXPECT_LT(std::abs(factor*(2.0f*map.from_value(16.0f) - 1.0f) - 1.0f), 1.0e-5f);
}
