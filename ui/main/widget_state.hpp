#ifndef TERRAFORMER_UI_MAIN_WIDGET_STATE_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_STATE_HPP

#include <cstdint>

namespace terraformer::ui::main
{
	enum class focus_indicator_mode:uint16_t{automatic, always_hidden, always_visible};

	struct widget_state
	{
		uint16_t collapsed:1;
		uint16_t hidden:1;
		uint16_t maximized:1;
		uint16_t disabled:1;
		uint16_t mbe_sensitive:1;
		uint16_t kbe_sensitive:1;
		focus_indicator_mode cursor_focus_indicator_mode:2;
		focus_indicator_mode kbd_focus_indicator_mode:2;

		constexpr bool interaction_is_disabled() const
		{ return disabled || hidden || collapsed; }

		constexpr bool accepts_mouse_input() const
		{ return !(disabled || hidden || collapsed) && mbe_sensitive; }

		constexpr bool accepts_keyboard_input() const
		{ return !(disabled || hidden || collapsed) && kbe_sensitive; }

		constexpr bool has_keyboard_focus_indicator() const
		{
			return (kbd_focus_indicator_mode == focus_indicator_mode::automatic && kbe_sensitive)
				|| kbd_focus_indicator_mode == focus_indicator_mode::always_visible;
		}

		constexpr bool has_cursor_focus_indicator() const
		{
			return (cursor_focus_indicator_mode == focus_indicator_mode::automatic && mbe_sensitive)
				|| cursor_focus_indicator_mode == focus_indicator_mode::always_visible;
		}
	};

	static_assert(sizeof(widget_state) == sizeof(uint16_t));

}

#endif
