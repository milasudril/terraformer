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

TESTCASE(terraformer_channel_mask_set_str_red)
{
	terraformer::channel_mask mask;
	mask.set("R");
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

TESTCASE(terraformer_channel_mask_set_str_green)
{
	terraformer::channel_mask mask;
	mask.set("G");
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

TESTCASE(terraformer_channel_mask_set_str_blue)
{
	terraformer::channel_mask mask;
	mask.set("B");
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

TESTCASE(terraformer_channel_mask_set_str_alpha)
{
	terraformer::channel_mask mask;
	mask.set("A");
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

TESTCASE(terraformer_channel_mask_set_str_luminance)
{
	terraformer::channel_mask mask;
	mask.set("Y");
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

TESTCASE(terraformer_channel_mask_set_str_junk)
{
	terraformer::channel_mask mask;
	mask.set("e!");
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

TESTCASE(terraformer_channel_mask_construct_red)
{
	terraformer::channel_mask mask{"R"};
	EXPECT_EQ(mask.has_red(), true);
	EXPECT_EQ(mask.has_green(), false);
	EXPECT_EQ(mask.has_blue(), false);
	EXPECT_EQ(mask.has_alpha(), false);
	EXPECT_EQ(mask.has_luminance(), false);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), false);
	EXPECT_EQ(mask.has_rgba(), false);
	EXPECT_EQ(mask.has("R"), true);
	EXPECT_EQ(mask.has("G"), false);
	EXPECT_EQ(mask.has("B"), false);
	EXPECT_EQ(mask.has("A"), false);
	EXPECT_EQ(mask.has("Y"), false);
	EXPECT_EQ(mask.has("er89e9r"), false);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(terraformer_channel_mask_construct_green)
{
	terraformer::channel_mask mask{"G"};
	EXPECT_EQ(mask.has_red(), false);
	EXPECT_EQ(mask.has_green(), true);
	EXPECT_EQ(mask.has_blue(), false);
	EXPECT_EQ(mask.has_alpha(), false);
	EXPECT_EQ(mask.has_luminance(), false);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), false);
	EXPECT_EQ(mask.has_rgba(), false);
	EXPECT_EQ(mask.has("R"), false);
	EXPECT_EQ(mask.has("G"), true);
	EXPECT_EQ(mask.has("B"), false);
	EXPECT_EQ(mask.has("A"), false);
	EXPECT_EQ(mask.has("Y"), false);
	EXPECT_EQ(mask.has("er89e9r"), false);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(terraformer_channel_mask_construct_blue)
{
	terraformer::channel_mask mask{"B"};
	EXPECT_EQ(mask.has_red(), false);
	EXPECT_EQ(mask.has_green(), false);
	EXPECT_EQ(mask.has_blue(), true);
	EXPECT_EQ(mask.has_alpha(), false);
	EXPECT_EQ(mask.has_luminance(), false);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), false);
	EXPECT_EQ(mask.has_rgba(), false);
	EXPECT_EQ(mask.has("R"), false);
	EXPECT_EQ(mask.has("G"), false);
	EXPECT_EQ(mask.has("B"), true);
	EXPECT_EQ(mask.has("A"), false);
	EXPECT_EQ(mask.has("Y"), false);
	EXPECT_EQ(mask.has("er89e9r"), false);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(terraformer_channel_mask_construct_alpha)
{
	terraformer::channel_mask mask{"A"};
	EXPECT_EQ(mask.has_red(), false);
	EXPECT_EQ(mask.has_green(), false);
	EXPECT_EQ(mask.has_blue(), false);
	EXPECT_EQ(mask.has_alpha(), true);
	EXPECT_EQ(mask.has_luminance(), false);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), false);
	EXPECT_EQ(mask.has_rgba(), false);
	EXPECT_EQ(mask.has("R"), false);
	EXPECT_EQ(mask.has("G"), false);
	EXPECT_EQ(mask.has("B"), false);
	EXPECT_EQ(mask.has("A"), true);
	EXPECT_EQ(mask.has("Y"), false);
	EXPECT_EQ(mask.has("er89e9r"), false);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(terraformer_channel_mask_construct_luminance)
{
	terraformer::channel_mask mask{"Y"};
	EXPECT_EQ(mask.has_red(), false);
	EXPECT_EQ(mask.has_green(), false);
	EXPECT_EQ(mask.has_blue(), false);
	EXPECT_EQ(mask.has_alpha(), false);
	EXPECT_EQ(mask.has_luminance(), true);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), false);
	EXPECT_EQ(mask.has_rgba(), false);
	EXPECT_EQ(mask.has("R"), false);
	EXPECT_EQ(mask.has("G"), false);
	EXPECT_EQ(mask.has("B"), false);
	EXPECT_EQ(mask.has("A"), false);
	EXPECT_EQ(mask.has("Y"), true);
	EXPECT_EQ(mask.has("er89e9r"), false);
	EXPECT_EQ(represents_grayscale_image(mask), true);
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(terraformer_channel_mask_construct_junk)
{
	terraformer::channel_mask mask{"X"};
	EXPECT_EQ(mask.has_red(), false);
	EXPECT_EQ(mask.has_green(), false);
	EXPECT_EQ(mask.has_blue(), false);
	EXPECT_EQ(mask.has_alpha(), false);
	EXPECT_EQ(mask.has_luminance(), false);
	EXPECT_EQ(mask.has_unsupported_channel(), true);
	EXPECT_EQ(mask.has_rgb(), false);
	EXPECT_EQ(mask.has_rgba(), false);
	EXPECT_EQ(mask.has("R"), false);
	EXPECT_EQ(mask.has("G"), false);
	EXPECT_EQ(mask.has("B"), false);
	EXPECT_EQ(mask.has("A"), false);
	EXPECT_EQ(mask.has("Y"), false);
	EXPECT_EQ(mask.has("X"), false);
	EXPECT_EQ(mask.has("er89e9r"), false);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(terraformer_channel_mask_set_red)
{
	terraformer::channel_mask mask;
	mask.set_red();
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

TESTCASE(terraformer_channel_mask_set_green)
{
	terraformer::channel_mask mask;
	mask.set_green();
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

TESTCASE(terraformer_channel_mask_set_blue)
{
	terraformer::channel_mask mask;
	mask.set_blue();
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

TESTCASE(terraformer_channel_mask_set_alpha)
{
	terraformer::channel_mask mask;
	mask.set_alpha();
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

TESTCASE(terraformer_channel_mask_set_luminance)
{
	terraformer::channel_mask mask;
	mask.set_luminance();
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

TESTCASE(terraformer_channel_mask_set_rgb)
{
	terraformer::channel_mask mask;
	mask.set_rgb();
	EXPECT_EQ(mask.has_red(), true);
	EXPECT_EQ(mask.has_green(), true);
	EXPECT_EQ(mask.has_blue(), true);
	EXPECT_EQ(mask.has_alpha(), false);
	EXPECT_EQ(mask.has_luminance(), false);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), true);
	EXPECT_EQ(mask.has_rgba(), false);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), true);
}

TESTCASE(terraformer_channel_mask_set_rgba)
{
	terraformer::channel_mask mask;
	mask.set_rgba();
	EXPECT_EQ(mask.has_red(), true);
	EXPECT_EQ(mask.has_green(), true);
	EXPECT_EQ(mask.has_blue(), true);
	EXPECT_EQ(mask.has_alpha(), true);
	EXPECT_EQ(mask.has_luminance(), false);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), true);
	EXPECT_EQ(mask.has_rgba(), true);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), true);
}

TESTCASE(terraformer_channel_mask_build)
{
	terraformer::channel_mask mask;

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

	mask.set("R");
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

	mask.set("G");
	EXPECT_EQ(mask.has_red(), true);
	EXPECT_EQ(mask.has_green(), true);
	EXPECT_EQ(mask.has_blue(), false);
	EXPECT_EQ(mask.has_alpha(), false);
	EXPECT_EQ(mask.has_luminance(), false);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), false);
	EXPECT_EQ(mask.has_rgba(), false);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), false);

	mask.set("B");
	EXPECT_EQ(mask.has_red(), true);
	EXPECT_EQ(mask.has_green(), true);
	EXPECT_EQ(mask.has_blue(), true);
	EXPECT_EQ(mask.has_alpha(), false);
	EXPECT_EQ(mask.has_luminance(), false);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), true);
	EXPECT_EQ(mask.has_rgba(), false);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), true);

	mask.set("A");
	EXPECT_EQ(mask.has_red(), true);
	EXPECT_EQ(mask.has_green(), true);
	EXPECT_EQ(mask.has_blue(), true);
	EXPECT_EQ(mask.has_alpha(), true);
	EXPECT_EQ(mask.has_luminance(), false);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), true);
	EXPECT_EQ(mask.has_rgba(), true);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), true);

	mask.set("Y");
	EXPECT_EQ(mask.has_red(), true);
	EXPECT_EQ(mask.has_green(), true);
	EXPECT_EQ(mask.has_blue(), true);
	EXPECT_EQ(mask.has_alpha(), true);
	EXPECT_EQ(mask.has_luminance(), true);
	EXPECT_EQ(mask.has_unsupported_channel(), false);
	EXPECT_EQ(mask.has_rgb(), true);
	EXPECT_EQ(mask.has_rgba(), true);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), false);

	mask.set("Dummy");
	EXPECT_EQ(mask.has_red(), true);
	EXPECT_EQ(mask.has_green(), true);
	EXPECT_EQ(mask.has_blue(), true);
	EXPECT_EQ(mask.has_alpha(), true);
	EXPECT_EQ(mask.has_luminance(), true);
	EXPECT_EQ(mask.has_unsupported_channel(), true);
	EXPECT_EQ(mask.has_rgb(), true);
	EXPECT_EQ(mask.has_rgba(), true);
	EXPECT_EQ(represents_grayscale_image(mask), false);
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(channel_mask_color_image_000)
{
	terraformer::channel_mask mask;
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(channel_mask_color_image_001)
{
	terraformer::channel_mask mask;
	mask.set_luminance();
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(channel_mask_color_image_with_010)
{
	terraformer::channel_mask mask;
	mask.set("Junk");
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(channel_mask_color_image_with_011)
{
	terraformer::channel_mask mask;
	mask.set("Junk").set_luminance();
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(channel_mask_color_image_100)
{
	terraformer::channel_mask mask;
	mask.set_rgb();
	EXPECT_EQ(represents_color_image(mask), true);
}

TESTCASE(channel_mask_color_image_101)
{
	terraformer::channel_mask mask;
	mask.set_rgb().set_luminance();
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(channel_mask_color_image_with_110)
{
	terraformer::channel_mask mask;
	mask.set_rgb().set("Junk");
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(channel_mask_color_image_with_111)
{
	terraformer::channel_mask mask;
	mask.set_rgb().set("Junk").set_luminance();
	EXPECT_EQ(represents_color_image(mask), false);
}

TESTCASE(channel_mask_grayscale_image_0000)
{
	terraformer::channel_mask mask;
	EXPECT_EQ(represents_grayscale_image(mask), false);
}

TESTCASE(channel_mask_grayscale_image_0001)
{
	terraformer::channel_mask mask;
	mask.set_alpha();
	EXPECT_EQ(represents_grayscale_image(mask), false);
}

TESTCASE(channel_mask_grayscale_image_0010)
{
	terraformer::channel_mask mask;
	mask.set_rgb();
	EXPECT_EQ(represents_grayscale_image(mask), false);
}

TESTCASE(channel_mask_grayscale_image_0011)
{
	terraformer::channel_mask mask;
	mask.set_rgb().set_alpha();
	EXPECT_EQ(represents_grayscale_image(mask), false);
}

TESTCASE(channel_mask_grayscale_image_0100)
{
	terraformer::channel_mask mask;
	mask.set("Junk");
	EXPECT_EQ(represents_grayscale_image(mask), false);
}

TESTCASE(channel_mask_grayscale_image_0101)
{
	terraformer::channel_mask mask;
	mask.set("Junk").set_alpha();
	EXPECT_EQ(represents_grayscale_image(mask), false);
}

TESTCASE(channel_mask_grayscale_image_0110)
{
	terraformer::channel_mask mask;
	mask.set("Junk").set_rgb();
	EXPECT_EQ(represents_grayscale_image(mask), false);
}

TESTCASE(channel_mask_grayscale_image_0111)
{
	terraformer::channel_mask mask;
	mask.set("Junk").set_rgb().set_alpha();
	EXPECT_EQ(represents_grayscale_image(mask), false);
}

TESTCASE(channel_mask_grayscale_image_1000)
{
	terraformer::channel_mask mask;
	mask.set_luminance();
	EXPECT_EQ(represents_grayscale_image(mask), true);
}

TESTCASE(channel_mask_grayscale_image_1001)
{
	terraformer::channel_mask mask;
	mask.set_luminance().set_alpha();
	EXPECT_EQ(represents_grayscale_image(mask), false);
}

TESTCASE(channel_mask_grayscale_image_1010)
{
	terraformer::channel_mask mask;
	mask.set_luminance().set_rgb();
	EXPECT_EQ(represents_grayscale_image(mask), false);
}

TESTCASE(channel_mask_grayscale_image_1011)
{
	terraformer::channel_mask mask;
	mask.set_luminance().set_rgb().set_alpha();
	EXPECT_EQ(represents_grayscale_image(mask), false);
}

TESTCASE(channel_mask_grayscale_image_1100)
{
	terraformer::channel_mask mask;
	mask.set_luminance().set("Junk");
	EXPECT_EQ(represents_grayscale_image(mask), false);
}

TESTCASE(channel_mask_grayscale_image_1101)
{
	terraformer::channel_mask mask;
	mask.set_luminance().set("Junk").set_alpha();
	EXPECT_EQ(represents_grayscale_image(mask), false);
}

TESTCASE(channel_mask_grayscale_image_1110)
{
	terraformer::channel_mask mask;
	mask.set_luminance().set("Junk").set_rgb();
	EXPECT_EQ(represents_grayscale_image(mask), false);
}

TESTCASE(channel_mask_grayscale_image_1111)
{
	terraformer::channel_mask mask;
	mask.set_luminance().set("Junk").set_rgb().set_alpha();
	EXPECT_EQ(represents_grayscale_image(mask), false);
}