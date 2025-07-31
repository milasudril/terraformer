//@	{"target":{"name":"rasterizer.test"}}

#include "./rasterizer.hpp"
#include "lib/common/spaces.hpp"
#include "testfwk/validation.hpp"

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
			[&callcount, item, pixel_size](auto x, auto y, auto&&) {
				auto const expected_loc = terraformer::location{0.5f, 0.5f, 0.0f}
					+ static_cast<float>(callcount)*item/2.0f;
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
		[&callcount, pixel_size](auto x, auto y, auto&&) {
			if(callcount < 8)
			{
				EXPECT_EQ(x, 0.5f + static_cast<float>(callcount));
				EXPECT_EQ(y, 0.5f);
			}
			else
			{
				EXPECT_EQ(y, 0.5f + static_cast<float>(callcount - 8 + 1));
			}
			fprintf(stderr, "%.8g %.8g\n", x, y);
			++callcount;
		}
	);
}