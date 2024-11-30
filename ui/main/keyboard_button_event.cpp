//@	{"target": {"name":"./keyboard_button_event.o", "rel":"implementation"}}

#include "./keyboard_button_event.hpp"

terraformer::ui::main::builtin_command_id terraformer::ui::main::to_builtin_command_id(
	keyboard_button_event const& kbe
)
{
	if(kbe.scancode == 0x39 && kbe.action == keyboard_button_action::press)
	{ return builtin_command_id::button_press; }

	if(kbe.scancode == 0x39 && kbe.action == keyboard_button_action::repeat)
	{ return builtin_command_id::button_press_repeat; }

	if(kbe.scancode == 0x39 && kbe.action == keyboard_button_action::release)
	{ return builtin_command_id::button_release; }

#if 0
		button_press,
		button_release,
		set_cursor_to_begin,
		set_cursor_to_end,
		move_cursor_left,
		move_cursor_right,
		select_to_begin,
		select_to_end,
		select_left,
		select_right,
		erase_backwards,
		erase_forwards,
		select_all
#endif
	return builtin_command_id::not_builtin;
}