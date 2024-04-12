//@	{"target":{"name":"whi.test"}}

#include "./whi.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_theming_whi_rgba_pixel_from_whi_inv)
{
	{
		auto const res = terraformer::ui::theming::make_rgba_pixel_from_whi_inv(0.0f, 1.0f);
		printf("%.8g %.8g %.8g\n", res.red(), res.green(), res.blue());
	}

	{
		auto const res = terraformer::ui::theming::make_rgba_pixel_from_whi_inv(1.0f/3.0f, 1.0f);
		printf("%.8g %.8g %.8g\n", res.red(), res.green(), res.blue());
	}

	{
		auto const res = terraformer::ui::theming::make_rgba_pixel_from_whi_inv(2.0f/3.0f, 1.0f);
		printf("%.8g %.8g %.8g\n", res.red(), res.green(), res.blue());
	}
}