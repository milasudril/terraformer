#ifndef TERRAFORMER_UI_MAIN_BUILDIN_COMMAND_ID_HPP
#define TERRAFORMER_UI_MAIN_BUILDIN_COMMAND_ID_HPP

namespace terraformer::ui::main
{
	enum class builtin_command_id
	{
		not_builtin,
		button_press,
		button_press_repeat,
		button_release,

		erase_backwards,
		erase_forwards,
		go_to_begin,
		select_to_begin,
		go_to_end,
		select_to_end,
		step_left,
		select_left,
		step_right,
		select_right,
		step_up,
		select_up,
		step_down,
		select_down,
		select_all,
		copy,
		cut,
		paste
	};
}

#endif