//@	{"target":{"name":"fixed_intensity_colormap.test"}}

//#include "./fixed_intensity_colormap.hpp"

#include <testfwk/testfwk.hpp>

#if 0
TESTCASE(terraformer_ui_theming_fixed_intensity_colormap_normalize)
{
	// TODO: Add verdict
	namespace fic = terraformer::ui::theming::fixed_intensity_colormap_helpers;
	{
		auto const val = fic::normalize(terraformer::rgba_pixel{1.0f, 0.0f, 0.0f});
		printf("%s\n", to_string(val).c_str());
		auto const dot = fic::weights*val.value();
		auto const i = dot[0] + dot[1] + dot[2];
		printf("%.8g\n", i);
	}

	{
		auto const val = fic::normalize(terraformer::rgba_pixel{0.0f, 1.0f, 0.0f});
		printf("%s\n", to_string(val).c_str());
		auto const dot = fic::weights*val.value();
		auto const i = dot[0] + dot[1] + dot[2];
		printf("%.8g\n", i);
	}
}

TESTCASE(terraformer_ui_theming_fixed_intensity_colormap_max_blue_compensate_with_other)
{
	// TODO: Add verdict
	namespace fic = terraformer::ui::theming::fixed_intensity_colormap_helpers;
	{
		auto const val = fic::max_blue_compensate_with_other(0.0f);
		printf("%s\n", to_string(val).c_str());
		auto const dot = fic::weights*val.value();
		auto const i = dot[0] + dot[1] + dot[2];
		printf("%.8g\n", i);
	}

	{
		auto const val = fic::max_blue_compensate_with_other(0.5f);
		printf("%s\n", to_string(val).c_str());
		auto const dot = fic::weights*val.value();
		auto const i = dot[0] + dot[1] + dot[2];
		printf("%.8g\n", i);
	}

	{
		auto const val = fic::max_blue_compensate_with_other(1.0f);
		printf("%s\n", to_string(val).c_str());
		auto const dot = fic::weights*val.value();
		auto const i = dot[0] + dot[1] + dot[2];
		printf("%.8g\n", i);
	}
}

TESTCASE(terraformer_ui_theming_fixed_intensity_colormap_get_lut)
{
	// TODO: Add verdict
	auto const& lut = terraformer::ui::theming::fixed_intensity_colormap::lut;
	for(size_t k = 0; k != std::size(lut); ++k)
	{
		printf("%s\n", to_string(lut[k]).c_str());
	}
}

TESTCASE(terraformer_ui_theming_fixed_intensity_colormap_call)
{
	// TODO: Add verdict
	auto const& lut = terraformer::ui::theming::fixed_intensity_colormap::lut;
	auto const n = 2*std::size(lut);
	for(size_t k = 0; k != n; ++k)
	{
		auto const t = static_cast<float>(k)/static_cast<float>(n);
		printf("%s\n", to_string(terraformer::ui::theming::fixed_intensity_colormap{}(t)).c_str());
	}
}
#endif