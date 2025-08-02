//@	{"target":{"name":"rasterizer.test"}}

#include "./rasterizer.hpp"
#include "lib/array_classes/single_array.hpp"
#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"
#include "testfwk/validation.hpp"

#include <random>
#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_visit_pixels_different_directions)
{
	std::array const displacements{
		terraformer::displacement{2.0f, 0.0f, 0.0f},
		terraformer::displacement{2.0f, 1.0f, 0.0f},
		terraformer::displacement{1.0f, 2.0f, 0.0f},
		terraformer::displacement{0.0f, 2.0f, 0.0f},
		terraformer::displacement{-1.0f, 2.0f, 0.0f},
		terraformer::displacement{-2.0f, 1.0f, 0.0f},
		terraformer::displacement{-2.0f, 0.0f, 0.0f},
		terraformer::displacement{-2.0f, -1.0f, 0.0f},
		terraformer::displacement{-1.0f, -2.0f, 0.0f},
		terraformer::displacement{0.0f, -2.0f, 0.0f},
		terraformer::displacement{1.0f, -2.0f, 0.0f},
		terraformer::displacement{2.0f, -1.0f, 0.0f}
	};

	for(auto const item :displacements)
	{
		std::array const locs{
			terraformer::location{},
			terraformer::location{} + item,
			terraformer::location{} + 2.0f*item
		};

		size_t callcount = 0;
		auto const pixel_size = 0.25f;
		visit_pixels(
			terraformer::span{std::begin(locs), std::end(locs)},
			pixel_size,
			[&callcount, item, pixel_size](auto x, auto y, auto&&...) {
				auto const expected_loc = static_cast<float>(callcount)*item/2.0f;
				EXPECT_EQ(x, expected_loc[0]);
				EXPECT_EQ(y, expected_loc[1]);
				++callcount;
			}
		);

		auto const expected_callcount = 2.0f*static_cast<float>(std::size(locs) - 1)/pixel_size;
		EXPECT_EQ(static_cast<float>(callcount), expected_callcount);
	};
}

TESTCASE(terraformer_visit_pixels_curve_through_non_integer_point_with_dir_change)
{
	std::array const locs{
		terraformer::location{},
		terraformer::location{3.75f, 0.0f, 0.0f},
		terraformer::location{2.25f, 1.75f, 0.0f},
		terraformer::location{0.25f, 5.75f, 0.0f}
	};

	size_t callcount = 0;
	auto const pixel_size = 0.5f;
	visit_pixels(
		terraformer::span{std::begin(locs), std::end(locs)},
		pixel_size,
		[&callcount, pixel_size](auto x, auto y, auto&&...) {
			if(callcount < 7)
			{
				EXPECT_EQ(x,  static_cast<float>(callcount));
				EXPECT_EQ(y, 0.0f);
			}
			else
			if(callcount < 10)
			{ EXPECT_EQ(y, static_cast<float>(callcount - 7)); }
			else
			{ EXPECT_EQ(y, 3.5f + static_cast<float>(callcount - 10)); }
			++callcount;
		}
	);
}

TESTCASE(terraformer_visit_pixels_curge_random_points)
{
	for(size_t k_max = 2; k_max <= 65536; k_max <<= 1)
	{
		terraformer::single_array<terraformer::location> locs;
		terraformer::random_generator rng;
		terraformer::location loc{};
		for(size_t k = 0; k != k_max; ++k)
		{
			locs.push_back(loc);

			std::uniform_real_distribution u{1.0f, 2.0f};
			auto sign_dist = std::uniform_int_distribution{0, 1};
			auto const sign = sign_dist(rng) == 0? -1.0f : 1.0f;
			terraformer::displacement const dr{sign*u(rng), sign*u(rng), 0.0f};
			loc += dr;
		}

		auto const pixel_size = 1.0f/16.0f;
		auto x_0 = 0;
		auto y_0 = 0;
		visit_pixels(
			locs,
			pixel_size,
			[&x_0, &y_0, callcount = 0](auto x, auto y, auto&&...) mutable{
				auto const x_int = static_cast<int>(std::round(x));
				auto const y_int = static_cast<int>(std::round(y));
				if(callcount != 0)
				{
					auto const dx_size = std::abs(x_int - x_0);
					auto const dy_size = std::abs(y_int - y_0);
					EXPECT_EQ(dx_size > 0 || dy_size > 0, true);
				}

				++callcount;
				x_0 = x_int;
				y_0 = y_int;
			}
		);

		auto const end = (locs.back() - terraformer::location{})/pixel_size;
		EXPECT_LT(std::abs(end[0] - static_cast<float>(x_0)), 2.5f);
		EXPECT_LT(std::abs(end[1] - static_cast<float>(y_0)), 2.5f);
	}
}