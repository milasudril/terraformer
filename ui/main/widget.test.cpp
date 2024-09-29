//@	{"target":{"name":"widget.test"}}

#include "./widget.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_main_widget_size_minimize_height)
{
	terraformer::ui::main::widget_size_constraints const size{
		.width = terraformer::ui::main::widget_size_range{32.0f, 256.0f},
		.height = terraformer::ui::main::widget_size_range{16.0f, 64.0f}
	};

	auto const scaling = minimize_height(size);
	EXPECT_EQ(scaling[0], 32.0f);
	EXPECT_EQ(scaling[1], 16.0f);
}

TESTCASE(terraformer_ui_main_widget_size_minimize_width)
{
	terraformer::ui::main::widget_size_constraints const size{
		.width = terraformer::ui::main::widget_size_range{32.0f, 256.0f},
		.height = terraformer::ui::main::widget_size_range{16.0f, 64.0f}
	};

	auto const scaling = minimize_width(size);
	EXPECT_EQ(scaling[0], 32.0f);
	EXPECT_EQ(scaling[1], 16.0f);
}
