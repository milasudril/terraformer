//@	{"target":{"name":"fixed_intensity_colormap.test"}}

#include "./fixed_intensity_colormap.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_theming_fixed_intensity_colormap_half_intensity)
{
	namespace theming = terraformer::ui::theming;
	theming::fixed_intensity_colormap cmap{terraformer::perceptual_color_intensity{0.5f}};
	for(size_t k = 0; k != 12; ++k)
	{
		auto const value = cmap(static_cast<float>(k)/12.0f);
		EXPECT_EQ(value.alpha(), 1.0f);
		EXPECT_EQ(
			terraformer::perceptual_color_intensity{value},
			terraformer::perceptual_color_intensity{0.5f}
		);
	}

	auto const cmap_bright = theming::fixed_intensity_colormap::make_pastels(
		cmap,
		terraformer::perceptual_color_intensity{1.0f}
	);
	for(size_t k = 0; k != 12; ++k)
	{
		auto const value = cmap_bright(static_cast<float>(k)/12.0f);
		EXPECT_EQ(value.alpha(), 1.0f);
		EXPECT_LT(
			std::abs(terraformer::perceptual_color_intensity{value} - terraformer::perceptual_color_intensity{1.0f}),
			1.0e-5f
		);
	}
}