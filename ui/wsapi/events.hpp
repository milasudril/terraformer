#ifndef TERRAFORMER_UI_WSAPI_EVENTS_HPP
#define TERRAFORMER_UI_WSAPI_EVENTS_HPP

#include "lib/common/bitmask_enum.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <string>

namespace terraformer::ui::wsapi
{
	struct fb_size
	{
		int width;
		int height;
	};

	struct cursor_position
	{
		double x;
		double y;
	};

	enum class button_action:int{press, release};

	enum class modifier_keys
	{
		shift = GLFW_MOD_SHIFT,
		control = GLFW_MOD_CONTROL,
		alt = GLFW_MOD_ALT,
		super = GLFW_MOD_SUPER,
		capslock = GLFW_MOD_CAPS_LOCK,
		numlock = GLFW_MOD_NUM_LOCK
	};

	consteval void enable_bitmask_operators(modifier_keys){}

	struct mouse_button_event
	{
		cursor_position where;
		int button;
		button_action action;
		modifier_keys modifiers;
	};

	struct cursor_motion_event
	{
		cursor_position where;
	};

	enum class cursor_enter_leave:int{enter, leave};

	struct cursor_enter_leave_event
	{
		cursor_position where;
		cursor_enter_leave direction;
	};

	struct error_message
	{
		std::string description;
	};

	struct typing_event
	{
		uint32_t codepoint;
	};
}
#endif