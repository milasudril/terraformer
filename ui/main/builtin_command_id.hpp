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
		go_to_begin,
		go_to_end,
		step_left,
		step_right,
		select_to_begin,
		select_to_end,
		select_left,
		select_right,
		erase_backwards,
		erase_forwards,
		select_all
	};
}

#endif