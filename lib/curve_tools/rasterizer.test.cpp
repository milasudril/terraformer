//@	{"target":{"name":"rasterizer.test"}}

#include "./rasterizer.hpp"

#include "lib/array_classes/single_array.hpp"
#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"
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

TESTCASE(terraformer_make_thick_curve_empty_input)
{
	auto res = make_thick_curve(
		terraformer::span<terraformer::location const>{},
		terraformer::span<float const>{}
	);

	EXPECT_EQ(res.data.empty(), true);
}

TESTCASE(terraformer_make_thick_curve_single_element_only)
{
	terraformer::location const loc{};
	auto const thickness = 4.0f;

	auto res = make_thick_curve(
		terraformer::span{&loc, &loc + 1},
		terraformer::span{&thickness, &thickness + 1}
	);

	EXPECT_EQ(res.data.empty(), true);
}

TESTCASE(terraformer_make_thick_curve_first_segment_broken)
{
	std::array locs{
		terraformer::location{},
		terraformer::location{0.0f, 1.0f, 0.0f},
		terraformer::location{1.0f, 1.0f, 0.0f},
		terraformer::location{2.0f, 1.333333f, 0.0f},
	};

	std::array thicknesses{
		2.0f,
		2.0f,
		2.0f,
		2.0f
	};

	auto res = make_thick_curve(
		terraformer::span{std::begin(locs), std::end(locs)},
		terraformer::span{std::begin(thicknesses), std::end(thicknesses)}
	);

	// First element was removed
	EXPECT_EQ(std::size(res.data).get(), 3);
	auto const locs_out = res.locations();
	auto const elems = res.data.element_indices();
	auto const running_lenghts = res.running_lengths();
	std::array locs_out_expected{
		terraformer::location{0.0f, 0.5f, 0.0f},
		terraformer::location{1.0f, 1.0f, 0.0f},
		terraformer::location{2.0f, 1.333333f, 0.0f},
	};
	std::array running_lenghts_expected{
		0.0f,
		distance(locs_out_expected[1], locs_out_expected[0]),
		  distance(locs_out_expected[1], locs_out_expected[0])
		+ distance(locs_out_expected[2], locs_out_expected[1]),
	};
	for(auto index : elems)
	{
		auto const i = index - elems.front();
		EXPECT_EQ(locs_out[index], locs_out_expected[i]);
		EXPECT_EQ(running_lenghts[index], running_lenghts_expected[i]);
	}
	EXPECT_EQ(res.curve_length, running_lenghts_expected.back());
}

TESTCASE(terraformer_make_thick_curve_all_segments_broken)
{
	std::array locs{
		terraformer::location{},
		terraformer::location{0.0f, 1.0f, 0.0f},
		terraformer::location{1.0f, 1.0f, 0.0f},
		terraformer::location{1.0f, 0.0f, 0.0f},
	};

	std::array thicknesses{
		2.0f,
		2.0f,
		2.0f,
		2.0f
	};

	auto res = make_thick_curve(
		terraformer::span{std::begin(locs), std::end(locs)},
		terraformer::span{std::begin(thicknesses), std::end(thicknesses)}
	);

	// Nothing interesting left
	EXPECT_EQ(std::size(res.data).get(), 0);
	EXPECT_EQ(res.curve_length, 0.0f);
}

TESTCASE(terraformer_make_thick_curve_only_two_vertices)
{
	std::array locs{
		terraformer::location{},
		terraformer::location{0.0f, 1.0f, 0.0f}
	};

	std::array thicknesses{
		2.0f,
		2.0f
	};

	auto res = make_thick_curve(
		terraformer::span{std::begin(locs), std::end(locs)},
		terraformer::span{std::begin(thicknesses), std::end(thicknesses)}
	);

	EXPECT_EQ(std::size(res.data).get(), 2);
	auto const elems = res.data.element_indices();
	auto const normals_out = res.normals();
	auto const locs_out = res.locations();
	for(auto index : elems)
	{
		EXPECT_EQ(locs_out[index], locs[index - elems.front()]);
		EXPECT_EQ(normals_out[index], (terraformer::direction{terraformer::displacement{1.0f, 0.0f, 0.0f}}));
	}
	EXPECT_EQ(res.curve_length, 1.0f);
}

TESTCASE(terraformer_make_thick_curve_in_the_middle_and_last)
{
	std::array locs{
		terraformer::location{},
		terraformer::location{0.0f, 1.0f, 0.0f},
		terraformer::location{0.5f, 2.0f, 0.0f},
		terraformer::location{1.5f, 1.5f, 0.0f},
	};

	std::array thicknesses{
		2.0f,
		2.0f,
		2.0f,
		2.0f
	};

	auto res = make_thick_curve(
		terraformer::span{std::begin(locs), std::end(locs)},
		terraformer::span{std::begin(thicknesses), std::end(thicknesses)}
	);
	EXPECT_EQ(std::size(res.data).get(), 2);

	auto const locs_out = res.locations();
	auto const elems = res.data.element_indices();
	auto const running_lenghts = res.running_lengths();
	std::array locs_out_expected{
		terraformer::location{},
		terraformer::location{0.875f, 1.5f, 0.0f}

	};
	std::array running_lenghts_expected{
		0.0f,
		distance(locs_out_expected[1], locs_out_expected[0])
	};
	for(auto index : elems)
	{
		auto const i = index - elems.front();
		EXPECT_EQ(locs_out[index], locs_out_expected[i]);
		EXPECT_EQ(running_lenghts[index], running_lenghts_expected[i]);
	}
	EXPECT_EQ(res.curve_length, running_lenghts_expected.back());
}

TESTCASE(terraformer_fill_curve_using_quads)
{
	terraformer::image output{512, 512};

	std::array<terraformer::location, 48> locs{};
	std::array<float, 48> thicknesses{};
	for(size_t k = 0; k != std::size(locs); ++k)
	{

		auto const theta = 2.0f*std::numbers::pi_v<float>*static_cast<float>(k)
			/static_cast<float>(std::size(locs));
		thicknesses[k] = (0.5f*std::sin(6.0f*theta) + 1.0f)/32.0f;
		locs[k] = terraformer::location{0.5f, 0.5f, 0.0f}
			+ 0.25f*terraformer::displacement{std::cos(theta), std::sin(theta), 0.0f};
	}

	auto curve = make_thick_curve(
		terraformer::span{std::begin(locs), std::end(locs)},
		terraformer::span{std::begin(thicknesses), std::end(thicknesses)}
	);
	EXPECT_EQ(std::size(curve.data).get(), 48);

	fill_using_quads(curve.attributes(), 1.0f/512.0f, output.pixels(),[length = curve.curve_length](terraformer::location loc){
		return terraformer::rgba_pixel{0.5f*(loc[0] + 1.0f), loc[1]/length, 0.0f, 1.0f};
	});

	store(output, std::format("{}/{}_fill_curve_using_quads.exr", MAIKE_BUILDINFO_TARGETDIR, MAIKE_TASKID).c_str());

}
