//@	{"target": {"name":"./keyboard_button_event.o", "rel":"implementation"}}

#include "./keyboard_button_event.hpp"

terraformer::ui::main::builtin_command_id terraformer::ui::main::to_builtin_command_id(
	keyboard_button_event const& kbe
)
{
	auto const shift_pressed = (kbe.modifiers & modifier_keys::shift) == modifier_keys::shift;
	auto const ctrl_pressed = (kbe.modifiers & modifier_keys::control) == modifier_keys::control;

	// Spacebar
	if(kbe.scancode == 0x39)
	{
		if(kbe.action == keyboard_button_action::press)
		{ return builtin_command_id::button_press; }

		if(kbe.action == keyboard_button_action::repeat)
		{ return builtin_command_id::button_press_repeat; }

		if(kbe.action == keyboard_button_action::release)
		{ return builtin_command_id::button_release; }
	}

	// Home key
	if(kbe.scancode == 0x66)
	{
		if(kbe.action == keyboard_button_action::press && !shift_pressed)
		{ return builtin_command_id::go_to_begin; }

		if(kbe.action == keyboard_button_action::press && shift_pressed)
		{ return builtin_command_id::select_to_begin; }
	}

	// End key
	if(kbe.scancode == 0x6b)
	{
		if(kbe.action == keyboard_button_action::press && !shift_pressed)
		{ return builtin_command_id::go_to_end; }

		if(kbe.action == keyboard_button_action::press && shift_pressed)
		{ return builtin_command_id::select_to_end; }
	}

	// Backspace
	if(
		kbe.scancode == 0xe &&
		(kbe.action == keyboard_button_action::press || kbe.action == keyboard_button_action::repeat)
	)
	{ return builtin_command_id::erase_backwards; }

	// Delete
	if(
		kbe.scancode == 0x6f &&
		(kbe.action == keyboard_button_action::press || kbe.action == keyboard_button_action::repeat)
	)
	{ return builtin_command_id::erase_forwards; }

	// Left arrow
	if(kbe.scancode == 0x69)
	{
		if(
			(kbe.action == keyboard_button_action::press || kbe.action == keyboard_button_action::repeat)
			&& !shift_pressed
		)
		{ return builtin_command_id::step_left; }

		if(
			(kbe.action == keyboard_button_action::press || kbe.action == keyboard_button_action::repeat)
			&& shift_pressed
		)
		{ return builtin_command_id::select_left; }
	}

	// Right arrow
	if(kbe.scancode == 0x6a)
	{
		if(
			(kbe.action == keyboard_button_action::press || kbe.action == keyboard_button_action::repeat)
			&& !shift_pressed
		)
		{ return builtin_command_id::step_right; }

		if(
			(kbe.action == keyboard_button_action::press || kbe.action == keyboard_button_action::repeat)
			&& shift_pressed
		)
		{ return builtin_command_id::select_right; }
	}

	// Up arrow
	if(kbe.scancode == 0x67)
	{
		if(
			(kbe.action == keyboard_button_action::press || kbe.action == keyboard_button_action::repeat)
			&& !shift_pressed
		)
		{ return builtin_command_id::step_up; }

		if(
			(kbe.action == keyboard_button_action::press || kbe.action == keyboard_button_action::repeat)
			&& shift_pressed
		)
		{ return builtin_command_id::select_up; }
	}

	// Down arrow
	if(kbe.scancode == 0x6c)
	{
		if(
			(kbe.action == keyboard_button_action::press || kbe.action == keyboard_button_action::repeat)
			&& !shift_pressed
		)
		{ return builtin_command_id::step_down; }

		if(
			(kbe.action == keyboard_button_action::press || kbe.action == keyboard_button_action::repeat)
			&& shift_pressed
		)
		{ return builtin_command_id::select_down; }
	}

	if(kbe.scancode == 0x1e)
	{
		if(kbe.action == keyboard_button_action::press && ctrl_pressed)
		{ return builtin_command_id::select_all; }
	}

	return builtin_command_id::not_builtin;
}