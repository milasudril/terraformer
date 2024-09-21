//@	{"target":{"name":"intensity.test"}}

#include "./intensity.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_intensity_different_weights)
{
	{
		using only_red = terraformer::intensity<1.0f, 0.0f, 0.0f>;
		EXPECT_EQ((only_red{terraformer::rgba_pixel{1.0, 0.0f, 0.0f}}), only_red{1.0f});
		EXPECT_EQ((only_red{terraformer::rgba_pixel{0.0, 1.0f, 0.0f}}), only_red{0.0f});
		EXPECT_EQ((only_red{terraformer::rgba_pixel{0.0, 0.0f, 1.0f}}), only_red{0.0f});
		EXPECT_EQ((only_red{terraformer::rgba_pixel{1.0, 1.0f, 1.0f}}), only_red{1.0f});
	}

	{
		using only_green = terraformer::intensity<0.0f, 1.0f, 0.0f>;
		EXPECT_EQ((only_green{terraformer::rgba_pixel{1.0, 0.0f, 0.0f}}), only_green{0.0f});
		EXPECT_EQ((only_green{terraformer::rgba_pixel{0.0, 1.0f, 0.0f}}), only_green{1.0f});
		EXPECT_EQ((only_green{terraformer::rgba_pixel{0.0, 0.0f, 1.0f}}), only_green{0.0f});
		EXPECT_EQ((only_green{terraformer::rgba_pixel{1.0, 1.0f, 1.0f}}), only_green{1.0f});
	}

	{
		using only_blue = terraformer::intensity<0.0f, 0.0f, 1.0f>;
		EXPECT_EQ((only_blue{terraformer::rgba_pixel{1.0, 0.0f, 0.0f}}), only_blue{0.0f});
		EXPECT_EQ((only_blue{terraformer::rgba_pixel{0.0, 1.0f, 0.0f}}), only_blue{0.0f});
		EXPECT_EQ((only_blue{terraformer::rgba_pixel{0.0, 0.0f, 1.0f}}), only_blue{1.0f});
		EXPECT_EQ((only_blue{terraformer::rgba_pixel{1.0, 1.0f, 1.0f}}), only_blue{1.0f});
	}

	{
		using all = terraformer::intensity<1.0f, 1.0f, 1.0f>;
		EXPECT_EQ((all{terraformer::rgba_pixel{1.0, 0.0f, 0.0f}}), all{1.0f});
		EXPECT_EQ((all{terraformer::rgba_pixel{0.0, 2.0f, 0.0f}}), all{2.0f});
		EXPECT_EQ((all{terraformer::rgba_pixel{0.0, 0.0f, 3.0f}}), all{3.0f});
		EXPECT_EQ((all{terraformer::rgba_pixel{1.0, 2.0f, 3.0f}}), all{6.0f});
	}

	{
		using all_non_uniform = terraformer::intensity<3.0f, 2.0f, 1.0f>;
		EXPECT_EQ((all_non_uniform{terraformer::rgba_pixel{1.0, 0.0f, 0.0f}}), all_non_uniform{3.0f});
		EXPECT_EQ((all_non_uniform{terraformer::rgba_pixel{0.0, 2.0f, 0.0f}}), all_non_uniform{4.0f});
		EXPECT_EQ((all_non_uniform{terraformer::rgba_pixel{0.0, 0.0f, 3.0f}}), all_non_uniform{3.0f});
		EXPECT_EQ((all_non_uniform{terraformer::rgba_pixel{1.0, 2.0f, 3.0f}}), all_non_uniform{10.0f});
	}
}

TESTCASE(terraformer_intensity_brighten)
{
	using intensity = terraformer::intensity<1.0f, 1.0f, 1.0f>;
	auto const res = brighten(terraformer::rgba_pixel{0.0f, 0.0f, 1.0f}, intensity{2.0f});
	EXPECT_EQ(intensity{res}, intensity{2.0f});
	EXPECT_EQ(res, (terraformer::rgba_pixel{0.5f, 0.5f, 1.0f}));
}

TESTCASE(terraformer_intensity_brighten_input_brighter_than_target)
{
	using intensity = terraformer::intensity<1.0f, 1.0f, 1.0f>;
	auto const res = brighten(terraformer::rgba_pixel{1.0f, 0.0f, 1.0f}, intensity{1.5f});
	EXPECT_EQ(intensity{res}, intensity{2.0f});
	EXPECT_EQ(res, (terraformer::rgba_pixel{1.0f, 0.0f, 1.0f}));
}

TESTCASE(terraformer_intensity_brighten_target_out_of_range)
{
	using intensity = terraformer::intensity<1.0f, 1.0f, 1.0f>;
	auto const res = brighten(terraformer::rgba_pixel{1.0f, 0.0f, 1.0f}, intensity{4.0f});
	EXPECT_EQ(intensity{res}, intensity{3.0f});
	EXPECT_EQ(res, (terraformer::rgba_pixel{1.0f, 1.0f, 1.0f}));
}

TESTCASE(terraformer_intensity_normalize_target_darker_than_max)
{
	using intensity = terraformer::intensity<1.0f, 1.0f, 1.0f>;
	auto const res = normalize(terraformer::rgba_pixel{0.5f, 0.0f, 0.0f}, intensity{1.0f});
	EXPECT_EQ(intensity{res}, intensity{1.0f});
	EXPECT_EQ(res.red(), 1.0f);
	EXPECT_EQ(res.green(), 0.0f);
	EXPECT_EQ(res.blue(), 0.0f);
}

TESTCASE(terraformer_intensity_normalize_target_brighter_than_max)
{
	using intensity = terraformer::intensity<1.0f, 1.0f, 1.0f>;
	auto const res = normalize(terraformer::rgba_pixel{0.5f, 0.0f, 0.0f}, intensity{2.0f});
	EXPECT_EQ(intensity{res}, intensity{2.0f});
	EXPECT_EQ(res.red(), 1.0f);
	EXPECT_EQ(res.green(), 0.5f);
	EXPECT_EQ(res.blue(), 0.5f);
}