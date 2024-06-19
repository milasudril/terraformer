//@	{"target":{"name": "button.test"}}

#include "./button.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ui_widgets_button_handle_mbe_press_release_button_0_value_false)
{
	terraformer::ui::widgets::button my_button;
	auto callcount = 0;

	my_button.on_activated([&callcount, &my_button](auto& button){
		++callcount;
		EXPECT_EQ(&button, &my_button);
	});

	my_button.handle_event(terraformer::ui::wsapi::fb_size{
		.width = 20,
		.height = 10
	});

	EXPECT_EQ(callcount, 0);
	EXPECT_EQ(my_button.value(), false);
	// TODO: Verify that button is rendered using "released" as background

	my_button.handle_event(terraformer::ui::wsapi::mouse_button_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.button = 0,
		.action = terraformer::ui::wsapi::button_action::press,
		.modifiers = {}
	});
	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(callcount, 0);
	// TODO: Verify that button is rendered using "pressed" as background

	my_button.handle_event(terraformer::ui::wsapi::mouse_button_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.button = 0,
		.action = terraformer::ui::wsapi::button_action::release,
		.modifiers = {}
	});
	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(callcount, 1);
	// TODO: Verify that button is rendered using "released" as background
}

TESTCASE(terraformer_ui_widgets_button_handle_mbe_press_release_button_0_value_true)
{
	terraformer::ui::widgets::button my_button;
	auto callcount = 0;

	my_button.on_activated([&callcount, &my_button](auto& button){
		++callcount;
		EXPECT_EQ(&button, &my_button);
	});

	my_button.handle_event(terraformer::ui::wsapi::fb_size{
		.width = 20,
		.height = 10
	});

	my_button.value(true);
	EXPECT_EQ(my_button.value(), true);
	EXPECT_EQ(callcount, 0);
	// TODO: Verify that button is rendered using "pressed" as background

	my_button.handle_event(terraformer::ui::wsapi::mouse_button_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.button = 0,
		.action = terraformer::ui::wsapi::button_action::press,
		.modifiers = {}
	});
	EXPECT_EQ(my_button.value(), true);
	EXPECT_EQ(callcount, 0);
	// TODO: Verify that button is rendered using "pressed" as background

	my_button.handle_event(terraformer::ui::wsapi::mouse_button_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.button = 0,
		.action = terraformer::ui::wsapi::button_action::release,
		.modifiers = {}
	});
	EXPECT_EQ(my_button.value(), true);
	EXPECT_EQ(callcount, 1);
	// TODO: Verify that button is rendered using "pressed" as background
}

TESTCASE(terraformer_ui_widgets_button_handle_mbe_release_button_0_no_action)
{
	terraformer::ui::widgets::button my_button;

	my_button.handle_event(terraformer::ui::wsapi::fb_size{
		.width = 20,
		.height = 10
	});

	my_button.handle_event(terraformer::ui::wsapi::mouse_button_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.button = 0,
		.action = terraformer::ui::wsapi::button_action::release,
		.modifiers = {}
	});
	EXPECT_EQ(my_button.value(), false);
	// TODO: Verify that button is rendered using "released" as background
}

TESTCASE(terraformer_ui_widgets_button_handle_mbe_press_button_1)
{
	terraformer::ui::widgets::button my_button;

	my_button.handle_event(terraformer::ui::wsapi::fb_size{
		.width = 20,
		.height = 10
	});

	auto callcount = 0;

	my_button.on_activated([&callcount, &my_button](auto& button){
		++callcount;
		EXPECT_EQ(&button, &my_button);
	});
	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(callcount, 0);
	// TODO: Verify that button is rendered using "released" as backgrround

	my_button.handle_event(terraformer::ui::wsapi::mouse_button_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.button = 1,
		.action = terraformer::ui::wsapi::button_action::press,
		.modifiers = {}
	});

	EXPECT_EQ(callcount, 0);
	// TODO: Verify that button is rendered using "released" as background
}

TESTCASE(terraformer_ui_widgets_button_handle_mbe_press_button_0_leave_and_enter_value_false)
{
	terraformer::ui::widgets::button my_button;
	auto callcount = 0;

	my_button.on_activated([&callcount, &my_button](auto& button){
		++callcount;
		EXPECT_EQ(&button, &my_button);
	});
	my_button.handle_event(terraformer::ui::wsapi::fb_size{
		.width = 20,
		.height = 10
	});
	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(callcount, 0);
	// TODO: Verify that button is rendered using "released" as background

	my_button.handle_event(terraformer::ui::wsapi::mouse_button_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.button = 0,
		.action = terraformer::ui::wsapi::button_action::press,
		.modifiers = {}
	});
	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(callcount, 0);
	// TODO: Verify that button is rendered using "pressed" as background

	my_button.handle_event(terraformer::ui::wsapi::cursor_enter_leave_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.direction = terraformer::ui::wsapi::cursor_enter_leave::leave
	});
	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(callcount, 0);
	// TODO: Verify that button is rendered using "released" as background

	my_button.handle_event(terraformer::ui::wsapi::cursor_enter_leave_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.direction = terraformer::ui::wsapi::cursor_enter_leave::enter
	});
	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(callcount, 0);
	// TODO: Verify that button is rendered using "released" as background
}

TESTCASE(terraformer_ui_widgets_button_handle_mbe_press_button_0_leave_and_enter_value_true)
{
	terraformer::ui::widgets::button my_button;
	auto callcount = 0;

	my_button.on_activated([&callcount, &my_button](auto& button){
		++callcount;
		EXPECT_EQ(&button, &my_button);
	})
	.value(true)
	.handle_event(terraformer::ui::wsapi::fb_size{
		.width = 20,
		.height = 10
	});

	EXPECT_EQ(my_button.value(), true);
	EXPECT_EQ(callcount, 0);
	// TODO: Verify that button is rendered using "pressed" as background

	my_button.handle_event(terraformer::ui::wsapi::mouse_button_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.button = 0,
		.action = terraformer::ui::wsapi::button_action::press,
		.modifiers = {}
	});
	EXPECT_EQ(my_button.value(), true);
	EXPECT_EQ(callcount, 0);
	// TODO: Verify that button is rendered using "pressed" as background

	my_button.handle_event(terraformer::ui::wsapi::cursor_enter_leave_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.direction = terraformer::ui::wsapi::cursor_enter_leave::leave
	});
	EXPECT_EQ(my_button.value(), true);
	EXPECT_EQ(callcount, 0);
	// TODO: Verify that button is rendered using "pressed" as background

	my_button.handle_event(terraformer::ui::wsapi::cursor_enter_leave_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.direction = terraformer::ui::wsapi::cursor_enter_leave::enter
	});
	EXPECT_EQ(my_button.value(), true);
	EXPECT_EQ(callcount, 0);
	// TODO: Verify that button is rendered using "pressed" as background
}

TESTCASE(terraformer_ui_widgets_button_handle_cme)
{
	terraformer::ui::widgets::button my_button;
	auto callcount = 0;

	my_button.on_activated([&callcount, &my_button](auto& button){
		++callcount;
		EXPECT_EQ(&button, &my_button);
	});
	EXPECT_EQ(callcount, 0);

	my_button.handle_event(terraformer::ui::wsapi::cursor_motion_event{});
}
