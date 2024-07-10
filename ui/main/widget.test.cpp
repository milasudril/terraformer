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

TESTCASE(terraformer_ui_main_input_device_grab)
{
	terraformer::ui::main::input_device_grab grab;

	EXPECT_EQ(grab.has_device(terraformer::ui::main::input_device_mask::default_keyboard), false);
	EXPECT_EQ(grab.has_device(terraformer::ui::main::input_device_mask::default_mouse), false);
}

#if 0
namespace
{
	struct dummy_widget
	{
		void handle_event(
			terraformer::ui::main::mouse_button_event const& mbe,
			terraformer::ui::main::input_device_grab& grab
		)
		{
			mbe_received = &mbe;
			grab_received = &grab;
		}

		void handle_event(
			terraformer::ui::main::cursor_motion_event const& cme,
			terraformer::ui::main::input_device_grab& grab
		)
		{
			cme_received = &cme;
			grab_received = &grab;
		}

		void handle_event(
			terraformer::ui::main::typing_event const& te,
			terraformer::ui::main::input_device_grab& grab
		)
		{
			te_received = &te;
			grab_received = &grab;
		}

		void handle_event(
			terraformer::ui::main::keyboard_button_event const& kbe,
			terraformer::ui::main::input_device_grab& grab
		)
		{
			kbe_receivecd = &kbe;
			grab_received = &grab;
		}

		terraformer::ui::main::mouse_button_event const* mbe_received = nullptr;
		terraformer::ui::main::cursor_motion_event const* cme_received = nullptr;
		terraformer::ui::main::typing_event const* te_received = nullptr;
		terraformer::ui::main::keyboard_button_event const* kbe_receivecd = nullptr;
		terraformer::ui::main::input_device_grab* grab_received = nullptr;
	};
}

TESTCASE(terraformer_ui_main_input_device_grab_grab_keyboard)
{
	dummy_widget my_widget;
	terraformer::ui::main::input_device_grab grab{
		my_widget,
		terraformer::ui::main::input_device_mask::default_keyboard
	};

	EXPECT_EQ(grab.has_device(terraformer::ui::main::input_device_mask::default_keyboard), true);
	EXPECT_EQ(grab.has_device(terraformer::ui::main::input_device_mask::default_mouse), false);

	terraformer::ui::main::typing_event te{};
	grab.handle_event(te);
	EXPECT_EQ(my_widget.mbe_received, nullptr);
	EXPECT_EQ(my_widget.cme_received, nullptr);
	EXPECT_EQ(my_widget.te_received, &te);
	EXPECT_EQ(my_widget.kbe_receivecd, nullptr);
	EXPECT_EQ(my_widget.grab_received, &grab);

	my_widget.te_received = nullptr;
	my_widget.grab_received = nullptr;

	terraformer::ui::main::keyboard_button_event kbe{};
	grab.handle_event(kbe);
	EXPECT_EQ(my_widget.mbe_received, nullptr);
	EXPECT_EQ(my_widget.cme_received, nullptr);
	EXPECT_EQ(my_widget.te_received, nullptr);
	EXPECT_EQ(my_widget.kbe_receivecd, &kbe);
	EXPECT_EQ(my_widget.grab_received, &grab);
}


TESTCASE(terraformer_ui_main_input_device_grab_grab_mouse)
{
	dummy_widget my_widget;
	terraformer::ui::main::input_device_grab grab{
		my_widget,
		terraformer::ui::main::input_device_mask::default_mouse
	};

	EXPECT_EQ(grab.has_device(terraformer::ui::main::input_device_mask::default_keyboard), false);
	EXPECT_EQ(grab.has_device(terraformer::ui::main::input_device_mask::default_mouse), true);

	terraformer::ui::main::mouse_button_event mbe{};
	grab.handle_event(mbe);
	EXPECT_EQ(my_widget.mbe_received, &mbe);
	EXPECT_EQ(my_widget.cme_received, nullptr);
	EXPECT_EQ(my_widget.te_received, nullptr);
	EXPECT_EQ(my_widget.kbe_receivecd, nullptr);
	EXPECT_EQ(my_widget.grab_received, &grab);

	my_widget.mbe_received = nullptr;
	my_widget.grab_received = nullptr;

	terraformer::ui::main::cursor_motion_event cme{};
	grab.handle_event(cme);
	EXPECT_EQ(my_widget.mbe_received, nullptr);
	EXPECT_EQ(my_widget.cme_received, &cme);
	EXPECT_EQ(my_widget.te_received, nullptr);
	EXPECT_EQ(my_widget.kbe_receivecd, nullptr);
	EXPECT_EQ(my_widget.grab_received, &grab);
}

TESTCASE(terraformer_ui_main_input_device_grab_grab_keyboard_and_mouse)
{
	dummy_widget my_widget;
	terraformer::ui::main::input_device_grab grab{
		my_widget,
		 terraformer::ui::main::input_device_mask::default_mouse
		|terraformer::ui::main::input_device_mask::default_keyboard
	};

	EXPECT_EQ(grab.has_device(terraformer::ui::main::input_device_mask::default_keyboard), true);
	EXPECT_EQ(grab.has_device(terraformer::ui::main::input_device_mask::default_mouse), true);
}
#endif