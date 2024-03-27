#ifndef TERRAFORMER_UI_MAIN_EVENT_DISPATCHER_HPP
#define TERRAFORMER_UI_MAIN_EVENT_DISPATCHER_HPP

#include "ui/wsapi/native_window.hpp"

namespace terraformer::ui::main
{
	template<class T>
	concept event_dispatcher = requires(
		T& obj,
		fb_size const& fb_size,
		cursor_position const& pos,
		mouse_button_event const& mbe
	)
	{
		{ obj.dispatch(fb_size) } -> std::same_as<void>;
		{ obj.dispatch(pos) } -> same_as<bool>;
		{ obj.dispatch(mbe) } -> same_as<bool>;
		{ obj.render_widgets() } -> same_as<void>;
	};
}

#endif