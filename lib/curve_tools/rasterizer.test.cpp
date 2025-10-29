//@	{"target":{"name":"rasterizer.test"}}

#include "./rasterizer.hpp"

#include "lib/array_classes/single_array.hpp"
#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/image_io.hpp"

#include "lib/pixel_store/rgba_pixel.hpp"
#include "testfwk/validation.hpp"

#include <cstdio>
#include <random>
#include <testfwk/testfwk.hpp>
#include <format>

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
			[&callcount, item, pixel_size](auto loc, auto...) {
				auto const expected_loc = callcount <= 8?
					static_cast<float>(callcount)*item/2.0f : static_cast<float>(callcount - 1)*item/2.0f;
				EXPECT_EQ(loc[0], expected_loc[0]);
				EXPECT_EQ(loc[1], expected_loc[1]);
				++callcount;
			}
		);

		auto const expected_callcount = 2.0f*static_cast<float>(std::size(locs) - 1)/pixel_size + 2;
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
		[&callcount, pixel_size](auto loc, auto ...) {
			auto const x = loc[0];
			auto const y = loc[1];
			if(callcount < 8)
			{
				EXPECT_EQ(x,  static_cast<float>(callcount));
				EXPECT_EQ(y, 0.0f);
			}
			else
			if(callcount < 11)
			{ EXPECT_EQ(y, static_cast<float>(callcount - 8)); }
			else
			if(callcount == 11)
			{ EXPECT_EQ(y, 3.0f);}
			else
			{ EXPECT_EQ(y, 2.5f + static_cast<float>(callcount - 11)); }
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
			[&x_0, &y_0, callcount = 0](auto loc, auto...) mutable{
				auto const x_int = static_cast<int>(std::round(loc[0]));
				auto const y_int = static_cast<int>(std::round(loc[1]));
				if(callcount != 0)
				{
					auto const dx_size = std::abs(x_int - x_0);
					auto const dy_size = std::abs(y_int - y_0);
					EXPECT_EQ(dx_size >= 0 || dy_size >= 0, true);
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

TESTCASE(terraformer_make_distance_field)
{
	std::array<terraformer::location, 13> locs{};
	for(size_t k = 0; k != std::size(locs); ++k)
	{
		auto const theta = 2.0f*std::numbers::pi_v<float>*static_cast<float>(k)
			/static_cast<float>(std::size(locs) - 1);
		locs[k] = terraformer::location{0.5f, 0.5f, 0.0f}
			+ 0.25f*terraformer::displacement{std::cos(theta), std::sin(theta), 0.0f};
	}

	auto const l_max = 0.5f*std::numbers::pi_v<float>;

	terraformer::image output{512, 512};
	auto const t_start = std::chrono::steady_clock::now();
	make_distance_field(
		terraformer::scanline_processing_job_info{
			.input_y_offset = 0,
			.total_height = output.height()
		},
		output.pixels(),
		terraformer::span{std::begin(locs), std::end(locs)},
		1.0f/512.0f,
		[l_max](auto item) {
			auto const t = item.curve_parameter/l_max;
			auto const r = (1.0f/32.0f)*(1.0f + 0.5f*std::cos(6.0f*2.0f*std::numbers::pi_v<float>*(t + 1.0f/12.0f)));
			auto const d = std::max(1.0f - item.distance/r, 0.0f);
			return terraformer::rgba_pixel{t, d, t, 1.0f};
		}
	);
	auto const t_end = std::chrono::steady_clock::now();

	printf("Duration = %.8g\n", std::chrono::duration<double>(t_end - t_start).count());

	store(
		output,
		std::format("{}/{}_make_distance_field.exr", MAIKE_BUILDINFO_TARGETDIR, MAIKE_TASKID).c_str()
	);
}

TESTCASE(terraformer_make_curve_mask)
{
	std::array<terraformer::location, 13> locs{};
	for(size_t k = 0; k != std::size(locs); ++k)
	{
		auto const theta = 2.0f*std::numbers::pi_v<float>*static_cast<float>(k)
			/static_cast<float>(std::size(locs) - 1);
		locs[k] = terraformer::location{0.5f, 0.5f, 0.0f}
			+ 0.25f*terraformer::displacement{std::cos(theta), std::sin(theta), 0.0f};
	}

	terraformer::grayscale_image output{512, 512};
	auto const t_start = std::chrono::steady_clock::now();
	make_curve_mask(
		output.pixels(),
		terraformer::span{std::begin(locs), std::end(locs)},
		1.0f/512.0f,
		3.0f/64.0f
	);
	auto const t_end = std::chrono::steady_clock::now();

	printf("Duration = %.8g\n", std::chrono::duration<double>(t_end - t_start).count());

	store(
		output,
		std::format("{}/{}_curve_mask.exr", MAIKE_BUILDINFO_TARGETDIR, MAIKE_TASKID).c_str()
	);
}