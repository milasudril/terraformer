//@	{"target":{"name": "button.test"}}

#include "./button.hpp"
#include "ui/font_handling/font_mapper.hpp"

#include "lib/pixel_store/image_io.hpp"
#include <testfwk/testfwk.hpp>

namespace
{
	template<int N>
	struct dummy_texture
	{
		terraformer::image img;
		void upload(terraformer::span_2d<terraformer::rgba_pixel const> pixels)
		{
			img = terraformer::image{pixels};
		}
	};
	
	template<class TextureType>
	struct output_rect
	{
		TextureType const* foreground;
		TextureType const* background;
		std::array<terraformer::rgba_pixel, 4> foreground_tints;
		std::array<terraformer::rgba_pixel, 4> background_tints;
		
		static auto create_texture()
		{ return TextureType{}; }
	};
	
	auto create_render_resources()
	{
		terraformer::ui::font_handling::font_mapper fonts;
 		auto const fontfile = fonts.get_path("sans-serif");

		terraformer::shared_any body_text{
			std::type_identity<terraformer::ui::font_handling::font>{},
			std::move(terraformer::ui::font_handling::font{fontfile.c_str()}.set_font_size(11))
		};

		terraformer::object_dict resources;
		resources.insert<terraformer::object_dict>(
			"ui", std::move(
				terraformer::object_dict{}
				.insert<terraformer::object_dict>("command_area", std::move(
					terraformer::object_dict{}
						.insert<terraformer::rgba_pixel>("background_tint", 0.125f, 0.125f, 0.125f, 1.0f)
						.insert<terraformer::rgba_pixel>("text_color", 1.0f, 0.0f, 0.0f, 1.0f)
						.insert_link("font", body_text)
						.insert<float>("background_intensity", 1.0f)
					)
				)
				.insert<unsigned int>("widget_inner_margin", 4)
				.insert<unsigned int>("3d_border_thickness", 2)
			)
		);
		return resources;
	}
	
	auto inspect_button_state(terraformer::span_2d<terraformer::rgba_pixel const> img)
	{
		auto const w = img.width();
		auto const h = img.height();
		if(img(0, 0).red() > img(w - 1, h - 1).red())
		{ return terraformer::ui::widgets::button::state::released; }
		else
		if(img(0, 0).red() < img(w - 1, h - 1).red())
		{ return terraformer::ui::widgets::button::state::pressed; }

		throw std::runtime_error{"Failed to detect button state"};
	}
};

TESTCASE(terraformer_ui_widgets_button_handle_mbe_press_release_button_0_value_false)
{
	terraformer::ui::widgets::button my_button;
	auto callcount = 0;

	my_button.on_activated([&callcount, &my_button](auto& button){
		++callcount;
		EXPECT_EQ(&button, &my_button);
	}).
	theme_updated(create_render_resources());

	my_button.handle_event(terraformer::ui::wsapi::fb_size{
		.width = 20,
		.height = 14
	});

	EXPECT_EQ(my_button.value(), false);
	auto const resources = create_render_resources();
	output_rect<dummy_texture<0>> rect{};
	my_button.prepare_for_presentation(rect, terraformer::ui::main::widget_instance_info{}, resources);
	EXPECT_EQ(callcount, 0);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::released
	);

	my_button.handle_event(terraformer::ui::wsapi::mouse_button_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.button = 0,
		.action = terraformer::ui::wsapi::button_action::press,
		.modifiers = {}
	});
	EXPECT_EQ(my_button.value(), false);
	my_button.prepare_for_presentation(rect, terraformer::ui::main::widget_instance_info{}, resources);
	EXPECT_EQ(callcount, 0);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::pressed
	);

	my_button.handle_event(terraformer::ui::wsapi::mouse_button_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.button = 0,
		.action = terraformer::ui::wsapi::button_action::release,
		.modifiers = {}
	});
	EXPECT_EQ(my_button.value(), false);
	my_button.prepare_for_presentation(rect, terraformer::ui::main::widget_instance_info{}, resources);
	EXPECT_EQ(callcount, 1);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::released
	);
}

TESTCASE(terraformer_ui_widgets_button_handle_mbe_press_release_button_0_value_true)
{
	terraformer::ui::widgets::button my_button;
	auto callcount = 0;

	my_button.on_activated([&callcount, &my_button](auto& button){
		++callcount;
		EXPECT_EQ(&button, &my_button);
	}).
	theme_updated(create_render_resources());

	my_button.handle_event(terraformer::ui::wsapi::fb_size{
		.width = 20,
		.height = 14
	});

	my_button.value(true);
	auto const resources = create_render_resources();
	output_rect<dummy_texture<0>> rect{};
	my_button.prepare_for_presentation(rect, terraformer::ui::main::widget_instance_info{}, resources);
	EXPECT_EQ(callcount, 0);
	EXPECT_EQ(my_button.value(), true);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::pressed
	);

	my_button.handle_event(terraformer::ui::wsapi::mouse_button_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.button = 0,
		.action = terraformer::ui::wsapi::button_action::press,
		.modifiers = {}
	});
	my_button.prepare_for_presentation(rect, terraformer::ui::main::widget_instance_info{}, resources);
	EXPECT_EQ(callcount, 0);
	EXPECT_EQ(my_button.value(), true);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::pressed
	);

	my_button.handle_event(terraformer::ui::wsapi::mouse_button_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.button = 0,
		.action = terraformer::ui::wsapi::button_action::release,
		.modifiers = {}
	});
	my_button.prepare_for_presentation(rect, terraformer::ui::main::widget_instance_info{}, resources);
	EXPECT_EQ(callcount, 1);
	EXPECT_EQ(my_button.value(), true);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::pressed
	);
}

TESTCASE(terraformer_ui_widgets_button_handle_mbe_release_button_0_no_action)
{
	terraformer::ui::widgets::button my_button;

	my_button.handle_event(terraformer::ui::wsapi::fb_size{
		.width = 20,
		.height = 14
	});
	my_button.theme_updated(create_render_resources());

	my_button.handle_event(terraformer::ui::wsapi::mouse_button_event{
		.where = terraformer::ui::wsapi::cursor_position{},
		.button = 0,
		.action = terraformer::ui::wsapi::button_action::release,
		.modifiers = {}
	});
	EXPECT_EQ(my_button.value(), false);
	auto const resources = create_render_resources();
	output_rect<dummy_texture<0>> rect{};
	my_button.prepare_for_presentation(rect, terraformer::ui::main::widget_instance_info{}, resources);
	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::released
	);
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
