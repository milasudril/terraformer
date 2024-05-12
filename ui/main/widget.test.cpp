//@	{"target":{"name":"widget.test"}}

#include "./widget.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_main_widget_size_minimize_height)
{
	terraformer::ui::main::widget_size_constraints const size{
		.width{
			.min = 32.0f,
			.max = 256.0f
		},
		.height{
			.min = 16.0f,
			.max = 64.0f
		},
		.aspect_ratio = std::nullopt
	};

	auto const scaling = minimize_height(size);
	EXPECT_EQ(scaling[0], 32.0f);
	EXPECT_EQ(scaling[1], 16.0f);
}

TESTCASE(terraformer_ui_main_widget_size_minimize_height_aspect_ratio_in_range)
{
	terraformer::ui::main::widget_size_constraints const size{
		.width{
			.min = 32.0f,
			.max = 256.0f
		},
		.height{
			.min = 16.0f,
			.max = 64.0f
		},
		.aspect_ratio = 8.0f
	};

	auto const scaling = minimize_height(size);
	EXPECT_EQ(scaling[0], 128.0f);
	EXPECT_EQ(scaling[1], 16.0f);
	EXPECT_GE(scaling[0], size.width.min);
	EXPECT_LE(scaling[0], size.width.max);
	EXPECT_GE(scaling[1], size.height.min);
	EXPECT_LE(scaling[1], size.height.max);
}

TESTCASE(terraformer_ui_main_widget_size_minimize_height_aspect_ratio_low_within_height_range)
{
	terraformer::ui::main::widget_size_constraints const size{
		.width{
			.min = 32.0f,
			.max = 256.0f
		},
		.height{
			.min = 16.0f,
			.max = 64.0f
		},
		.aspect_ratio = 1.0f
	};

	auto const scaling = minimize_height(size);
	EXPECT_EQ(scaling[0], 32.0f);
	EXPECT_EQ(scaling[1], 32.0f);
	EXPECT_GE(scaling[0], size.width.min);
	EXPECT_LE(scaling[0], size.width.max);
	EXPECT_GE(scaling[1], size.height.min);
	EXPECT_LE(scaling[1], size.height.max);
}

TESTCASE(terraformer_ui_main_widget_size_minimize_height_aspect_ratio_high_within_height_range)
{
	terraformer::ui::main::widget_size_constraints const size{
		.width{
			.min = 32.0f,
			.max = 256.0f
		},
		.height{
			.min = 16.0f,
			.max = 64.0f
		},
		.aspect_ratio = 16.0f
	};

	auto const scaling = minimize_height(size);
	EXPECT_EQ(scaling[0], 256.0f);
	EXPECT_EQ(scaling[1], 16.0f);
	EXPECT_GE(scaling[0], size.width.min);
	EXPECT_LE(scaling[0], size.width.max);
	EXPECT_GE(scaling[1], size.height.min);
	EXPECT_LE(scaling[1], size.height.max);
}

TESTCASE(terraformer_ui_main_widget_size_minimize_height_aspect_ratio_too_low)
{
	terraformer::ui::main::widget_size_constraints const size{
		.width{
			.min = 32.0f,
			.max = 256.0f
		},
		.height{
			.min = 16.0f,
			.max = 64.0f
		},
		.aspect_ratio = 0.25f
	};

	try
	{
		[[maybe_unused]] auto const scaling = minimize_height(size);
		abort();
	}
	catch(...)
	{}
}

TESTCASE(terraformer_ui_main_widget_size_minimize_height_aspect_ratio_too_heigh)
{
	terraformer::ui::main::widget_size_constraints const size{
		.width{
			.min = 32.0f,
			.max = 256.0f
		},
		.height{
			.min = 16.0f,
			.max = 64.0f
		},
		.aspect_ratio = 32.0f
	};

	try
	{
		[[maybe_unused]] auto const scaling = minimize_height(size);
		abort();
	}
	catch(...)
	{}
}
///


TESTCASE(terraformer_ui_main_widget_size_minimize_width)
{
	terraformer::ui::main::widget_size_constraints const size{
		.width{
			.min = 32.0f,
			.max = 256.0f
		},
		.height{
			.min = 16.0f,
			.max = 64.0f
		},
		.aspect_ratio = std::nullopt
	};

	auto const scaling = minimize_width(size);
	EXPECT_EQ(scaling[0], 32.0f);
	EXPECT_EQ(scaling[1], 16.0f);
}

TESTCASE(terraformer_ui_main_widget_size_minimize_width_aspect_ratio_in_range)
{
	terraformer::ui::main::widget_size_constraints const size{
		.width{
			.min = 32.0f,
			.max = 256.0f
		},
		.height{
			.min = 16.0f,
			.max = 64.0f
		},
		.aspect_ratio = 8.0f
	};

	auto const scaling = minimize_width(size);
	EXPECT_EQ(scaling[0], 128.0f);
	EXPECT_EQ(scaling[1], 16.0f);
	EXPECT_GE(scaling[0], size.width.min);
	EXPECT_LE(scaling[0], size.width.max);
	EXPECT_GE(scaling[1], size.height.min);
	EXPECT_LE(scaling[1], size.height.max);
}

TESTCASE(terraformer_ui_main_widget_size_minimize_width_aspect_ratio_low_within_height_range)
{
	terraformer::ui::main::widget_size_constraints const size{
		.width{
			.min = 32.0f,
			.max = 256.0f
		},
		.height{
			.min = 16.0f,
			.max = 64.0f
		},
		.aspect_ratio = 1.0f
	};

	auto const scaling = minimize_width(size);
	EXPECT_EQ(scaling[0], 32.0f);
	EXPECT_EQ(scaling[1], 32.0f);
	EXPECT_GE(scaling[0], size.width.min);
	EXPECT_LE(scaling[0], size.width.max);
	EXPECT_GE(scaling[1], size.height.min);
	EXPECT_LE(scaling[1], size.height.max);
}

TESTCASE(terraformer_ui_main_widget_size_minimize_width_aspect_ratio_high_within_height_range)
{
	terraformer::ui::main::widget_size_constraints const size{
		.width{
			.min = 32.0f,
			.max = 256.0f
		},
		.height{
			.min = 16.0f,
			.max = 64.0f
		},
		.aspect_ratio = 16.0f
	};

	auto const scaling = minimize_width(size);
	EXPECT_EQ(scaling[0], 256.0f);
	EXPECT_EQ(scaling[1], 16.0f);
	EXPECT_GE(scaling[0], size.width.min);
	EXPECT_LE(scaling[0], size.width.max);
	EXPECT_GE(scaling[1], size.height.min);
	EXPECT_LE(scaling[1], size.height.max);
}

TESTCASE(terraformer_ui_main_widget_size_minimize_width_aspect_ratio_too_low)
{
	terraformer::ui::main::widget_size_constraints const size{
		.width{
			.min = 32.0f,
			.max = 256.0f
		},
		.height{
			.min = 16.0f,
			.max = 64.0f
		},
		.aspect_ratio = 0.25f
	};

	try
	{
		[[maybe_unused]] auto const scaling = minimize_width(size);
		abort();
	}
	catch(...)
	{}
}

TESTCASE(terraformer_ui_main_widget_size_minimize_width_aspect_ratio_too_heigh)
{
	terraformer::ui::main::widget_size_constraints const size{
		.width{
			.min = 32.0f,
			.max = 256.0f
		},
		.height{
			.min = 16.0f,
			.max = 64.0f
		},
		.aspect_ratio = 32.0f
	};

	try
	{
		[[maybe_unused]] auto const scaling = minimize_width(size);
		abort();
	}
	catch(...)
	{}
}