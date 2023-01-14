//@	{"target":{"name":"channel_mask.test"}}

#include "./channel_mask.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_channel_mask_default_state)
{
	terraformer::channel_mask const mask;
	EXPECT_EQ(mask.has_red(), false);
	EXPECT_EQ(mask.has_green(), false);
	EXPECT_EQ(mask.has_blue(), false);
	EXPECT_EQ(mask.has_alpha(), false);
	EXPECT_EQ(mask.has_luminance(), false);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), false);
	EXPECT_EQ(mask.has_rgba(), false);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(terraformer_channel_mask_construct_red)
{
	terraformer::channel_mask const mask{"R"};
	EXPECT_EQ(mask.has_red(), true);
	EXPECT_EQ(mask.has_green(), false);
	EXPECT_EQ(mask.has_blue(), false);
	EXPECT_EQ(mask.has_alpha(), false);
	EXPECT_EQ(mask.has_luminance(), false);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), false);
	EXPECT_EQ(mask.has_rgba(), false);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(terraformer_channel_mask_construct_green)
{
	terraformer::channel_mask const mask{"G"};
	EXPECT_EQ(mask.has_red(), false);
	EXPECT_EQ(mask.has_green(), true);
	EXPECT_EQ(mask.has_blue(), false);
	EXPECT_EQ(mask.has_alpha(), false);
	EXPECT_EQ(mask.has_luminance(), false);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), false);
	EXPECT_EQ(mask.has_rgba(), false);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(terraformer_channel_mask_construct_blue)
{
	terraformer::channel_mask const mask{"B"};
	EXPECT_EQ(mask.has_red(), false);
	EXPECT_EQ(mask.has_green(), false);
	EXPECT_EQ(mask.has_blue(), true);
	EXPECT_EQ(mask.has_alpha(), false);
	EXPECT_EQ(mask.has_luminance(), false);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), false);
	EXPECT_EQ(mask.has_rgba(), false);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(terraformer_channel_mask_construct_alpha)
{
	terraformer::channel_mask const mask{"A"};
	EXPECT_EQ(mask.has_red(), false);
	EXPECT_EQ(mask.has_green(), false);
	EXPECT_EQ(mask.has_blue(), false);
	EXPECT_EQ(mask.has_alpha(), true);
	EXPECT_EQ(mask.has_luminance(), false);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), false);
	EXPECT_EQ(mask.has_rgba(), false);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(terraformer_channel_mask_construct_luminance)
{
	terraformer::channel_mask const mask{"Y"};
	EXPECT_EQ(mask.has_red(), false);
	EXPECT_EQ(mask.has_green(), false);
	EXPECT_EQ(mask.has_blue(), false);
	EXPECT_EQ(mask.has_alpha(), false);
	EXPECT_EQ(mask.has_luminance(), true);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), false);
	EXPECT_EQ(mask.has_rgba(), false);
	EXPECT_EQ(represents_grayscale_image(mask), true);
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(terraformer_channel_mask_construct_junk)
{
	terraformer::channel_mask const mask{"X"};
	EXPECT_EQ(mask.has_red(), false);
	EXPECT_EQ(mask.has_green(), false);
	EXPECT_EQ(mask.has_blue(), false);
	EXPECT_EQ(mask.has_alpha(), false);
	EXPECT_EQ(mask.has_luminance(), false);
	EXPECT_EQ(mask.has_unsupported_channel(), true);
	EXPECT_EQ(mask.has_rgb(), false);
	EXPECT_EQ(mask.has_rgba(), false);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), false);
}