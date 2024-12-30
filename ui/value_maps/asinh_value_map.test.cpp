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


TESTCASE(terraformer_ui_value_maps_asinh_value_map_do_stuff)
{
	terraformer::ui::value_maps::asinh_value_map map{1500.0f, 10000.0f};
	auto const x0 = map.from_value(0.0f);
	auto const x1 = map.from_value(map.reference_value());
	auto const N = 10;
	auto const dx = (x1 - x0)*0.5f;
	for(size_t k = 0; k != N; ++k)
	{
		auto const a = std::min(static_cast<float>(k)*dx + x0 - 0.5f*dx, 1.0f);
		auto const b = std::min(static_cast<float>(k)*dx + x0 + 0.5f*dx, 1.0f);
		printf("%.8g %.8g\n", map.to_value(a), map.to_value(b));
	}

}