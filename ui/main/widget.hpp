#ifndef TERRAFORMER_UI_MAIN_WIDGET_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_HPP

#include "ui/wsapi/events.hpp"

#include <utility>
#include <type_traits>

namespace terraformer::ui::main
{
	struct widget_geometry
	{
		int x;
		int y;
		int width;
		int height;

		[[nodiscard]] constexpr bool operator==(widget_geometry const&) const = default;
		[[nodiscard]] constexpr bool operator!=(widget_geometry const&) const = default;
	};

	[[nodiscard]] inline bool inside(wsapi::cursor_position pos, widget_geometry const& box)
	{
		return (pos.x >= box.x && pos.x < box.x + box.width)
			&& (pos.y >= box.y && pos.y < box.y + box.height);
	}

	enum class widget_visibility:int{visible, not_rendered, collapsed};

	template<class T, class DrawingSurface>
	concept widget = requires(
		T& obj,
		wsapi::fb_size size,
		wsapi::cursor_position pos,
		wsapi::mouse_button_event const& mbe,
		DrawingSurface& surface
	)
	{
		{ obj.render() } -> std::same_as<void>;
		{ std::as_const(obj).background() } -> std::convertible_to<DrawingSurface>;
		{ std::as_const(obj).foreground() } -> std::convertible_to<DrawingSurface>;
		{ obj.handle_event(std::as_const(pos)) } -> std::same_as<bool>;
		{ obj.handle_event(mbe) } -> std::same_as<bool>;
		{ obj.handle_event(std::as_const(size)) } -> std::same_as<wsapi::fb_size>;
	};

	template<class RenderSurface>
	struct widget_with_default_actions
	{
		[[nodiscard]] RenderSurface background() const { return RenderSurface{}; }
		[[nodiscard]] RenderSurface foreground() const { return RenderSurface{}; }
		void render() const {}
		[[nodiscard]] bool handle_event(wsapi::cursor_position) const { return false; }
		[[nodiscard]] bool handle_event(wsapi::mouse_button_event const&) const { return false; }
		[[nodiscard]] wsapi::fb_size handle_event(wsapi::fb_size size) const { return size; }
	};

	static_assert(widget<widget_with_default_actions<int>, int>);
}

#endif
