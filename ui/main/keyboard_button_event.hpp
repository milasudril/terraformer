//@	{"dependencies_extra":[{"ref":"./keyboard_button_event.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_MAIN_KEYBOARD_BUTTON_EVENT_HPP
#define TERRAFORMER_UI_MAIN_KEYBOARD_BUTTON_EVENT_HPP

#include "./builtin_command_id.hpp"

namespace terraformer::ui::main
{
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

	enum class keyboard_button_action:int
	{
		press,
		release,
		repeat
	};

	struct keyboard_button_event
	{
		int scancode;
		keyboard_button_action action;
		modifier_keys modifiers;
	};

	builtin_command_id to_builtin_command_id(keyboard_button_event const& event);
}

#endif