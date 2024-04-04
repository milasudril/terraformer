#ifndef TERRAFORMER_UI_WSAPI_EVENTS_HPP
#define TERRAFORMER_UI_WSAPI_EVENTS_HPP

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

	enum class button_state_change:int{press, release};

	struct mouse_button_event
	{
		cursor_position where;
		int button;
		button_state_change state_change;
	};

	struct error_message
	{
		std::string description;
	};
}
#endif