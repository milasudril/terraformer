//@	{"target":{"name": "asinh_value_map.test"}}

#include "./asinh_value_map.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_value_maps_asinh_value_map_do_stuff)
{
	terraformer::ui::value_maps::asinh_value_map map{
	//	487.5161865999319f,
	//	0.6672893674556575f*6.0f
		490.0f,
		4.0f
	};

	EXPECT_EQ(map.max(), -map.min());
	EXPECT_EQ(map.max(), map.to_value(1.0f));
	EXPECT_EQ(map.min(), map.to_value(0.0f));

	EXPECT_EQ(std::round(6.0f*2.0f*(map.from_value(0.0f) - 0.5f)), 0.0f);
	EXPECT_EQ(std::round(6.0f*2.0f*(map.from_value(350.0f) - 0.5f)), 1.0f);
	EXPECT_EQ(std::round(6.0f*2.0f*(map.from_value(860.0f) - 0.5f)), 2.0f);
	EXPECT_EQ(std::round(6.0f*2.0f*(map.from_value(1800.0f) - 0.5f)), 3.0f);
	EXPECT_EQ(std::round(6.0f*2.0f*(map.from_value(3500.0f) - 0.5f)), 4.0f);
	EXPECT_EQ(std::round(6.0f*2.0f*(map.from_value(6800.0f) - 0.5f)), 5.0f);

	EXPECT_LT(std::abs(map.from_value(map.to_value(1.0f)) - 1.0f), 1.0e-5f);
	EXPECT_LT(std::abs(map.to_value(map.from_value(1000.0f)) - 1000.0f), 1.0e-4f);
}