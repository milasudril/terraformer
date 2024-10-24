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

	struct dummy_window
	{
		std::string title;
	};

	struct dummy_ui_controller
	{
		terraformer::ui::main::config cfg;

		void theme_updated(terraformer::ui::main::config&& new_config)
		{
			cfg = std::move(new_config);
		}
	};

	template<class TextureType>
	struct output_rect
	{
		using texture_type = TextureType;

		auto set_background(texture_type const* texture)
		{
			if(texture == nullptr)
			{ return terraformer::ui::main::set_texture_result::incompatible; }

			background = texture;
			return terraformer::ui::main::set_texture_result::success;
		}

		auto set_foreground(texture_type const* texture)
		{
			if(texture == nullptr)
			{ return terraformer::ui::main::set_texture_result::incompatible; }

			foreground = texture;
			return terraformer::ui::main::set_texture_result::success;
		}

		void set_background_tints(std::array<terraformer::rgba_pixel, 4> const& vals)
		{ background_tints = vals; }

		void set_foreground_tints(std::array<terraformer::rgba_pixel, 4> const& vals)
		{ foreground_tints = vals; }

		TextureType const* background;
		TextureType const* foreground;

		std::array<terraformer::rgba_pixel, 4> background_tints;
		std::array<terraformer::rgba_pixel, 4> foreground_tints;

		static auto create_texture()
		{ return TextureType{}; }
	};

	auto create_render_resources()
	{
		terraformer::ui::font_handling::font_mapper fonts;
 		auto const fontfile = fonts.get_path("sans-serif");
		auto const font = std::make_shared<terraformer::ui::font_handling::font>(fontfile.c_str());
		font->set_font_size(11);

		terraformer::ui::main::config resources;

		resources.command_area = terraformer::ui::main::widget_look{
			.colors{
				.background = terraformer::rgba_pixel{0.125f, 0.125f, 0.125f, 1.0f},
				.foreground = terraformer::rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
				.selection_color = terraformer::rgba_pixel{0.0f, 1.0f, 0.0f, 1.0f}
			},
			.padding{4.0f},
			.border_thickness = 2.0f,
			.font = font
		};

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

template<>
struct terraformer::ui::main::window_traits<dummy_window>
{
	static void set_title(dummy_window& window, char const* new_title)
	{
		window.title = new_title;
	}
};

TESTCASE(terraformer_ui_widgets_button_handle_mbe_press_release_button_0_value_false)
{
	terraformer::ui::widgets::button my_button;
	auto callcount = 0;

	dummy_window window{};
	dummy_ui_controller ui_ctrl{};

	my_button.on_activated([&callcount, &my_button, &window, &ui_ctrl](
		auto& button,
		terraformer::ui::main::window_ref win_ref,
		terraformer::ui::main::ui_controller ui_ctrl_ref
	){
		++callcount;
		EXPECT_EQ(&button, &my_button);
		EXPECT_EQ(win_ref.handle(), &window);
		EXPECT_EQ(ui_ctrl_ref.handle(), &ui_ctrl);
	}).
	theme_updated(create_render_resources(), terraformer::ui::main::widget_instance_info{});

	my_button.handle_event(terraformer::ui::main::fb_size{
		.width = 20,
		.height = 14
	});

	EXPECT_EQ(my_button.value(), false);
	output_rect<dummy_texture<0>> rect{};
	my_button.prepare_for_presentation(terraformer::ui::main::widget_rendering_result{std::ref(rect)});
	EXPECT_EQ(callcount, 0);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::released
	);

	my_button.handle_event(
		terraformer::ui::main::mouse_button_event{
			.where = terraformer::ui::main::cursor_position{},
			.button = 0,
			.action = terraformer::ui::main::mouse_button_action::press,
			.modifiers = {}
		},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);

	EXPECT_EQ(my_button.value(), false);
	my_button.prepare_for_presentation(terraformer::ui::main::widget_rendering_result{std::ref(rect)});
	EXPECT_EQ(callcount, 0);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::pressed
	);

	my_button.handle_event(
		terraformer::ui::main::mouse_button_event{
			.where = terraformer::ui::main::cursor_position{},
			.button = 0,
			.action = terraformer::ui::main::mouse_button_action::release,
			.modifiers = {}
		},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);

	EXPECT_EQ(my_button.value(), false);
	my_button.prepare_for_presentation(terraformer::ui::main::widget_rendering_result{std::ref(rect)});
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
	dummy_window window{};
	dummy_ui_controller ui_ctrl{};

	my_button.on_activated([&callcount, &my_button, &window, &ui_ctrl](
		auto& button,
		terraformer::ui::main::window_ref win_ref,
		terraformer::ui::main::ui_controller ui_ctrl_ref
	){
		++callcount;
		EXPECT_EQ(&button, &my_button);
		EXPECT_EQ(win_ref.handle(), &window);
		EXPECT_EQ(ui_ctrl_ref.handle(), &ui_ctrl);
	}).
	theme_updated(create_render_resources(), terraformer::ui::main::widget_instance_info{});

	my_button.handle_event(terraformer::ui::main::fb_size{
		.width = 20,
		.height = 14
	});

	my_button.value(true);
	output_rect<dummy_texture<0>> rect{};
	my_button.prepare_for_presentation(terraformer::ui::main::widget_rendering_result{std::ref(rect)});
	EXPECT_EQ(callcount, 0);
	EXPECT_EQ(my_button.value(), true);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::pressed
	);


	my_button.handle_event(
		terraformer::ui::main::mouse_button_event{
			.where = terraformer::ui::main::cursor_position{},
			.button = 0,
			.action = terraformer::ui::main::mouse_button_action::press,
			.modifiers = {}
		},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);

	my_button.prepare_for_presentation(terraformer::ui::main::widget_rendering_result{std::ref(rect)});
	EXPECT_EQ(callcount, 0);
	EXPECT_EQ(my_button.value(), true);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::pressed
	);

	my_button.handle_event(
		terraformer::ui::main::mouse_button_event{
			.where = terraformer::ui::main::cursor_position{},
			.button = 0,
			.action = terraformer::ui::main::mouse_button_action::release,
			.modifiers = {}
		},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);
	my_button.prepare_for_presentation(terraformer::ui::main::widget_rendering_result{std::ref(rect)});
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
	dummy_window window{};
	dummy_ui_controller ui_ctrl{};

	my_button.handle_event(terraformer::ui::main::fb_size{
		.width = 20,
		.height = 14
	});
	my_button.theme_updated(create_render_resources(), terraformer::ui::main::widget_instance_info{});

	my_button.handle_event(
		terraformer::ui::main::mouse_button_event{
			.where = terraformer::ui::main::cursor_position{},
			.button = 0,
			.action = terraformer::ui::main::mouse_button_action::release,
			.modifiers = {}
		},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);
	EXPECT_EQ(my_button.value(), false);
	output_rect<dummy_texture<0>> rect{};
	my_button.prepare_for_presentation(terraformer::ui::main::widget_rendering_result{std::ref(rect)});
	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::released
	);
}

TESTCASE(terraformer_ui_widgets_button_handle_mbe_press_button_1)
{
	terraformer::ui::widgets::button my_button;
	dummy_window window{};
	dummy_ui_controller ui_ctrl{};

	my_button.handle_event(terraformer::ui::main::fb_size{
		.width = 20,
		.height = 14
	});

	auto callcount = 0;

	my_button.on_activated([&callcount, &my_button, &window, &ui_ctrl](
		auto& button,
		terraformer::ui::main::window_ref win_ref,
		terraformer::ui::main::ui_controller ui_ctrl_ref
	){
		++callcount;
		EXPECT_EQ(&button, &my_button);
		EXPECT_EQ(win_ref.handle(), &window);
		EXPECT_EQ(ui_ctrl_ref.handle(), &ui_ctrl);
	})
	.theme_updated(create_render_resources(), terraformer::ui::main::widget_instance_info{});
	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(callcount, 0);

	auto const resources = create_render_resources();
	output_rect<dummy_texture<0>> rect{};
	my_button.prepare_for_presentation(terraformer::ui::main::widget_rendering_result{std::ref(rect)});
	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::released
	);

	my_button.handle_event(
		terraformer::ui::main::mouse_button_event{
			.where = terraformer::ui::main::cursor_position{},
			.button = 1,
			.action = terraformer::ui::main::mouse_button_action::press,
			.modifiers = {}
		},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);

	my_button.prepare_for_presentation(terraformer::ui::main::widget_rendering_result{std::ref(rect)});
	EXPECT_EQ(callcount, 0);
	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::released
	);
}

TESTCASE(terraformer_ui_widgets_button_handle_mbe_press_button_0_leave_and_enter_value_false)
{
	terraformer::ui::widgets::button my_button;
	auto callcount = 0;
	dummy_window window{};
	dummy_ui_controller ui_ctrl{};

	my_button.on_activated([&callcount, &my_button, &window, &ui_ctrl](
		auto& button,
		terraformer::ui::main::window_ref win_ref,
		terraformer::ui::main::ui_controller ui_ctrl_ref
	){
		++callcount;
		EXPECT_EQ(&button, &my_button);
		EXPECT_EQ(win_ref.handle(), &window);
		EXPECT_EQ(ui_ctrl_ref.handle(), &ui_ctrl);
	})
	.theme_updated(create_render_resources(),  terraformer::ui::main::widget_instance_info{});
	my_button.handle_event(terraformer::ui::main::fb_size{
		.width = 20,
		.height = 14
	});
	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(callcount, 0);
	output_rect<dummy_texture<0>> rect{};

	my_button.handle_event(
		terraformer::ui::main::mouse_button_event{
			.where = terraformer::ui::main::cursor_position{},
			.button = 0,
			.action = terraformer::ui::main::mouse_button_action::press,
			.modifiers = {}
		},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);
	my_button.prepare_for_presentation(terraformer::ui::main::widget_rendering_result{std::ref(rect)});
	EXPECT_EQ(callcount, 0);
	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::pressed
	);

	my_button.handle_event(
		terraformer::ui::main::cursor_leave_event{
			.where = terraformer::ui::main::cursor_position{},
		},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);
	my_button.prepare_for_presentation(terraformer::ui::main::widget_rendering_result{std::ref(rect)});
	EXPECT_EQ(callcount, 0);
	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::released
	);

	my_button.handle_event(
		terraformer::ui::main::cursor_enter_event{
			.where = terraformer::ui::main::cursor_position{}
		},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);
	my_button.prepare_for_presentation(terraformer::ui::main::widget_rendering_result{std::ref(rect)});
	EXPECT_EQ(callcount, 0);
	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::released
	);
}


TESTCASE(terraformer_ui_widgets_button_handle_mbe_press_button_0_leave_and_enter_value_true)
{
	terraformer::ui::widgets::button my_button;
	auto callcount = 0;
	dummy_window window{};
	dummy_ui_controller ui_ctrl{};

	my_button.on_activated([&callcount, &my_button, &window, &ui_ctrl](
		auto& button,
		terraformer::ui::main::window_ref win_ref,
		terraformer::ui::main::ui_controller ui_ctrl_ref
	){
		++callcount;
		EXPECT_EQ(&button, &my_button);
		EXPECT_EQ(win_ref.handle(), &window);
		EXPECT_EQ(ui_ctrl_ref.handle(), &ui_ctrl);
	})
	.value(true)
	.theme_updated(create_render_resources(), terraformer::ui::main::widget_instance_info{});

	my_button.handle_event(terraformer::ui::main::fb_size{
		.width = 20,
		.height = 14
	});

	output_rect<dummy_texture<0>> rect{};
	my_button.prepare_for_presentation(terraformer::ui::main::widget_rendering_result{std::ref(rect)});
	EXPECT_EQ(my_button.value(), true);
	EXPECT_EQ(callcount, 0);
		EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::pressed
	);

	my_button.handle_event(
		terraformer::ui::main::mouse_button_event{
			.where = terraformer::ui::main::cursor_position{},
			.button = 0,
			.action = terraformer::ui::main::mouse_button_action::press,
			.modifiers = {}
		},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);
	my_button.prepare_for_presentation(terraformer::ui::main::widget_rendering_result{std::ref(rect)});
	EXPECT_EQ(my_button.value(), true);
	EXPECT_EQ(callcount, 0);
	EXPECT_EQ(
		inspect_button_state(rect.background->img.pixels()),
		terraformer::ui::widgets::button::state::pressed
	);
	my_button.handle_event(
		terraformer::ui::main::cursor_leave_event{
			.where = terraformer::ui::main::cursor_position{},
		},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);
	my_button.prepare_for_presentation(terraformer::ui::main::widget_rendering_result{std::ref(rect)});
	EXPECT_EQ(my_button.value(), true);
	EXPECT_EQ(callcount, 0);
	my_button.handle_event(
		terraformer::ui::main::cursor_leave_event{
			.where = terraformer::ui::main::cursor_position{}
		},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);
	my_button.handle_event(
		terraformer::ui::main::cursor_enter_event{
			.where = terraformer::ui::main::cursor_position{}
		},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);
	my_button.prepare_for_presentation(terraformer::ui::main::widget_rendering_result{std::ref(rect)});
	EXPECT_EQ(my_button.value(), true);
	my_button.handle_event(
		terraformer::ui::main::cursor_leave_event{
			.where = terraformer::ui::main::cursor_position{}
		},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);
	EXPECT_EQ(callcount, 0);
}

TESTCASE(terraformer_ui_widgets_button_handle_cme)
{
	terraformer::ui::widgets::button my_button;
	auto callcount = 0;
	dummy_window window{};
	dummy_ui_controller ui_ctrl{};

	my_button.on_activated([&callcount, &my_button, &window, &ui_ctrl](
		auto& button,
		terraformer::ui::main::window_ref win_ref,
		terraformer::ui::main::ui_controller ui_ctrl_ref
	){
		++callcount;
		EXPECT_EQ(&button, &my_button);
		EXPECT_EQ(win_ref.handle(), &window);
		EXPECT_EQ(ui_ctrl_ref.handle(), &ui_ctrl);
	});

	my_button.handle_event(
		terraformer::ui::main::cursor_motion_event{},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);
	EXPECT_EQ(callcount, 0);
}

TESTCASE(terraformer_ui_widgets_button_compute_size_constraints)
{
	terraformer::ui::widgets::button my_button;
	auto callcount = 0;

	my_button.on_activated([&callcount, &my_button](
		auto& button,
		terraformer::ui::main::window_ref,
		terraformer::ui::main::ui_controller
	){
		++callcount;
		EXPECT_EQ(&button, &my_button);
	})
	.text(u8"")
	.theme_updated(create_render_resources(), terraformer::ui::main::widget_instance_info{});

	auto const res_a = my_button.compute_size(terraformer::ui::main::widget_width_request{});

	my_button.text(u8"Hello, World");
	auto const res_b = my_button.compute_size(terraformer::ui::main::widget_width_request{});

	EXPECT_LT(res_a[0], res_b[0]);

	auto const res_c = my_button.compute_size(terraformer::ui::main::widget_width_request{});
	EXPECT_EQ(res_b[0], res_c[0]);
	EXPECT_EQ(callcount, 0);
}

TESTCASE(terraformer_ui_widgets_toggle_button_on_value_changed)
{
	terraformer::ui::widgets::toggle_button my_button{};
	auto callcount = 0;
	dummy_window window{};
	dummy_ui_controller ui_ctrl{};

	my_button.on_value_changed([&callcount, &my_button, &window, &ui_ctrl](
		auto& button,
		terraformer::ui::main::window_ref win_ref,
		terraformer::ui::main::ui_controller ui_ctrl_ref
	){
		++callcount;
		EXPECT_EQ(&button, &my_button);
		EXPECT_EQ(win_ref.handle(), &window);
		EXPECT_EQ(ui_ctrl_ref.handle(), &ui_ctrl);
	});

	EXPECT_EQ(my_button.value(), false);
	EXPECT_EQ(callcount, 0);

	{
		decltype(auto) res = my_button.value(true);
		static_assert(std::is_same_v<decltype(res), terraformer::ui::widgets::toggle_button&>);
		EXPECT_EQ(callcount, 0);
	}

	my_button.handle_event(
		terraformer::ui::main::mouse_button_event{
			.where = terraformer::ui::main::cursor_position{},
			.button = 0,
			.action = terraformer::ui::main::mouse_button_action::press,
			.modifiers = {}
		},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);
	EXPECT_EQ(callcount, 0);

	my_button.handle_event(
		terraformer::ui::main::mouse_button_event{
			.where = terraformer::ui::main::cursor_position{},
			.button = 0,
			.action = terraformer::ui::main::mouse_button_action::release,
			.modifiers = {}
		},
		terraformer::ui::main::window_ref{window},
		terraformer::ui::main::ui_controller{ui_ctrl}
	);
	EXPECT_EQ(callcount, 1);
}

TESTCASE(terraformer_ui_widgets_toggle_button_text)
{
	terraformer::ui::widgets::toggle_button my_button{};
	auto callcount = 0;
	my_button.on_value_changed([&callcount, &my_button](auto& button, terraformer::ui::main::window_ref,
		terraformer::ui::main::ui_controller){
		++callcount;
		EXPECT_EQ(&button, &my_button);
	});

	EXPECT_EQ(my_button.value(), false);
	{
		decltype(auto) res = my_button.text(u8"Foobar");
		static_assert(std::is_same_v<decltype(res), terraformer::ui::widgets::toggle_button&>);
	}
	EXPECT_EQ(callcount, 0);
	EXPECT_EQ(my_button.value(), false);
}