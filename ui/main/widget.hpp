#ifndef TERRAFORMER_UI_MAIN_WIDGET_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_HPP

#include "ui/wsapi/native_window.hpp"

namespace terraformer::ui::main
{
	template<class T, class RenderSurface>
	concept widget = requires(
		T& obj,
		wsapi::fb_size size,
		wsapi::cursor_position pos,
		wsapi::mouse_button_event const& mbe,
		RenderSurface& surface
	)
	{
		{ std::as_const(obj).render_to(surface) } -> std::same_as<void>;
		{ obj.handle_event(std::as_const(pos)) } -> std::same_as<bool>;
		{ obj.handle_event(mbe) } -> std::same_as<bool>;
		{ obj.handle_event(std::as_const(size)) } -> std::same_as<wsapi::fb_size>;
	};

	template<class RenderSurface>
	struct widget_with_default_actions
	{
		void render_to(RenderSurface&) const {}
		bool handle_event(wsapi::cursor_position) const { return false; }
		bool handle_event(wsapi::mouse_button_event const&) const { return false; }
		wsapi::fb_size handle_event(wsapi::fb_size size) const { return size; }
	};

	static_assert(widget<widget_with_default_actions<int>, int>);
}

#endif