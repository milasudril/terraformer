//@	{"target":{"name":"trigfunc.test"}}

#include "./trigfunc.hpp"
#include "lib/common/spaces.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_approx_sine)
{
	auto const dx = 2.0f*std::numbers::pi_v<float>/12.0f;
	for(int k = -24; k != 24; ++k)
	{
		auto const x = dx*static_cast<float>(k);
		printf("%.8g %.8g\n", x, terraformer::approx_sine(x));
	}
}


TESTCASE(terraformer_distance_from_origin_to_edge_xy_box)
{
	{
		terraformer::box_size const b{2.0f, 1.0f, 0.0f};
		auto constexpr pi = std::numbers::pi_v<float>;

		for(size_t k = 0; k != 4; ++k)
		{
			auto const r = terraformer::distance_from_origin_to_edge_xy(b, pi/4.0f + (static_cast<float>(k) - 2.0f)*pi/2.0f);
			EXPECT_LT(std::abs(r - 1.0f/std::sqrt(2.0f)), 1.0e-8f);
		}

		EXPECT_EQ(terraformer::distance_from_origin_to_edge_xy(b, 0.0f), 1.0f);
		EXPECT_EQ(terraformer::distance_from_origin_to_edge_xy(b, pi), 1.0f);
		EXPECT_EQ(terraformer::distance_from_origin_to_edge_xy(b, -pi/2.0f), 0.5f);
		EXPECT_EQ(terraformer::distance_from_origin_to_edge_xy(b, pi/2.0f), 0.5f);
	}

	{
		terraformer::box_size const b{1.0f, 2.0f, 0.0f};
		auto constexpr pi = std::numbers::pi_v<float>;

		for(size_t k = 0; k != 4; ++k)
		{
			auto const r = terraformer::distance_from_origin_to_edge_xy(b, pi/4.0f + (static_cast<float>(k) - 2.0f)*pi/2.0f);
			EXPECT_LT(std::abs(r - 1.0f/std::sqrt(2.0f)), 1.0e-8f);
		}

		EXPECT_EQ(terraformer::distance_from_origin_to_edge_xy(b, 0.0f), 0.5f);
		EXPECT_EQ(terraformer::distance_from_origin_to_edge_xy(b, pi), 0.5f);
		EXPECT_EQ(terraformer::distance_from_origin_to_edge_xy(b, -pi/2.0f), 1.0f);
		EXPECT_EQ(terraformer::distance_from_origin_to_edge_xy(b, pi/2.0f), 1.0f);
	}
}