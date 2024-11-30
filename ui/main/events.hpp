#ifndef TERRAFORMER_UI_MAIN_EVENTS_HPP
#define TERRAFORMER_UI_MAIN_EVENTS_HPP

#include "./keyboard_button_event.hpp"

#include <string>
#include <cstdint>

namespace terraformer::ui::main
{
	struct fb_size
	{
		int width;
		int height;

		constexpr bool operator==(fb_size const&) const = default;
		constexpr bool operator!=(fb_size const&) const = default;
	};

	struct cursor_position
	{
		double x;
		double y;
	};

	enum class mouse_button_action:int{press, release};

	struct mouse_button_event
	{
		cursor_position where;
		int button;
		mouse_button_action action;
		modifier_keys modifiers;
	};

	struct cursor_motion_event
	{
		cursor_position where;
	};

	struct cursor_enter_event
	{
		cursor_position where;
	};

	struct cursor_leave_event
	{
		cursor_position where;
	};

	struct error_message
	{
		std::string description;
	};

	struct window_close_event
	{};

	struct typing_event
	{
		char32_t codepoint;
	};

	struct keyboard_focus_enter_event
	{};

	struct keyboard_focus_leave_event
	{};
}
#endif