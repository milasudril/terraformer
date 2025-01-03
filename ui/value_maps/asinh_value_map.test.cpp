//@	{"target":{"name": "asinh_value_map.test"}}

#include "./asinh_value_map.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_value_maps_asinh_value_map_do_stuff)
{
	terraformer::ui::value_maps::asinh_value_map map{
	//	246.9570874946463f,
	//	0.7403470212280202f*6.0f
		317.5461997790359f,
		0.6901447774480382f*6.0f
	};

	EXPECT_EQ(map.max(), -map.min());
	EXPECT_EQ(map.max(), map.to_value(1.0f));
	EXPECT_EQ(map.min(), map.to_value(0.0f));

	EXPECT_EQ(std::round(6.0f*2.0f*(map.from_value(0.0f) - 0.5f)), 0.0f);
	EXPECT_EQ(std::round(6.0f*2.0f*(map.from_value(200.0f) - 0.5f)), 1.0f);
	EXPECT_EQ(std::round(6.0f*2.0f*(map.from_value(600.0f) - 0.5f)), 2.0f);
	EXPECT_EQ(std::round(6.0f*2.0f*(map.from_value(1500.0f) - 0.5f)), 3.0f);
	EXPECT_EQ(std::round(6.0f*2.0f*(map.from_value(2500.0f) - 0.5f)), 4.0f);
	EXPECT_EQ(std::round(6.0f*2.0f*(map.from_value(5000.0f) - 0.5f)), 5.0f);

	EXPECT_LT(std::abs(map.from_value(map.to_value(1.0f)) - 1.0f), 1.0e-5f);
	EXPECT_LT(std::abs(map.to_value(map.from_value(500.0f)) - 500.0f), 1.0e-4f);

	printf("%.8g\n", 6.0f*2.0f*(map.from_value(0.0f) - 0.5f));
	printf("%.8g\n", 6.0f*2.0f*(map.from_value(200.0f) - 0.5f));
	printf("%.8g\n", 6.0f*2.0f*(map.from_value(600.0f) - 0.5f));
	printf("%.8g\n", 6.0f*2.0f*(map.from_value(1500.0f) - 0.5f));
	printf("%.8g\n", 6.0f*2.0f*(map.from_value(2500.0f) - 0.5f));
	printf("%.8g\n", 6.0f*2.0f*(map.from_value(5000.0f) - 0.5f));
	puts("===========");
	printf("%.8g\n", map.to_value(0.0f + 0.5f));
	printf("%.8g\n", map.to_value(1.0f/12.0f + 0.5f));
	printf("%.8g\n", map.to_value(2.0f/12.0f + 0.5f));
	printf("%.8g\n", map.to_value(3.0f/12.0f + 0.5f));
	printf("%.8g\n", map.to_value(4.0f/12.0f + 0.5f));
	printf("%.8g\n", map.to_value(5.0f/12.0f + 0.5f));
}