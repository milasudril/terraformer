#ifndef TERRAFORMER_UI_WSAPI_EVENTS_HPP
#define TERRAFORMER_UI_WSAPI_EVENTS_HPP

#include "lib/common/bitmask_enum.hpp"
#include <string>

namespace terraformer::ui::main
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

	enum class mouse_button_action:int{press, release};

	enum class modifier_keys
	{
		none = 0x0,
		shift = 0x1,
		control = 0x2,
		alt = 0x4,
		super = 0x8,
		capslock = 0x10,
		numlock = 0x20
	};

	consteval void enable_bitmask_operators(modifier_keys){}

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
}
#endif
