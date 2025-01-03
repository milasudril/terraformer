//@	{"target":{"name": "asinh_value_map.test"}}

#include "./asinh_value_map.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_value_maps_asinh_value_map_do_stuff)
{
	constexpr auto num_stops = 6.0f;

	terraformer::ui::value_maps::asinh_value_map map{
		266.3185546307779f,
		0.7086205026374324f*num_stops
	};

	EXPECT_EQ(map.max(), -map.min());
	EXPECT_EQ(map.max(), map.to_value(1.0f));
	EXPECT_EQ(map.min(), map.to_value(0.0f));

	EXPECT_EQ(std::round(num_stops*2.0f*(map.from_value(0.0f) - 0.5f)), 0.0f);
	EXPECT_EQ(std::round(num_stops*2.0f*(map.from_value(200.0f) - 0.5f)), 1.0f);
	EXPECT_EQ(std::round(num_stops*2.0f*(map.from_value(520.0f) - 0.5f)), 2.0f);
	EXPECT_EQ(std::round(num_stops*2.0f*(map.from_value(1100.0f) - 0.5f)), 3.0f);
	EXPECT_EQ(std::round(num_stops*2.0f*(map.from_value(2300.0f) - 0.5f)), 4.0f);
	EXPECT_EQ(std::round(num_stops*2.0f*(map.from_value(4600.0f) - 0.5f)), 5.0f);

	EXPECT_LT(std::abs(map.from_value(map.to_value(1.0f)) - 1.0f), 1.0e-5f);
	EXPECT_LT(std::abs(map.to_value(map.from_value(500.0f)) - 500.0f), 1.0e-4f);

	printf("%.8g\n", num_stops*2.0f*(map.from_value(0.0f) - 0.5f));
	printf("%.8g\n", num_stops*2.0f*(map.from_value(200.0f) - 0.5f));
	printf("%.8g\n", num_stops*2.0f*(map.from_value(520.0f) - 0.5f));
	printf("%.8g\n", num_stops*2.0f*(map.from_value(1100.0f) - 0.5f));
	printf("%.8g\n", num_stops*2.0f*(map.from_value(2300.0f) - 0.5f));
	printf("%.8g\n", num_stops*2.0f*(map.from_value(4600.0f) - 0.5f));
	puts("===========");

	for(size_t k = 0; k < static_cast<size_t>(num_stops); ++k)
	{
		printf("%.8g\n", map.to_value(static_cast<float>(k)/(2.0f*num_stops) + 0.5f));
	}
}