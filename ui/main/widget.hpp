#ifndef TERRAFORMER_UI_MAIN_WIDGET_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_HPP

#include "ui/wsapi/events.hpp"
#include "ui/theming/widget_look.hpp"
#include "lib/common/spaces.hpp"

#include <utility>
#include <type_traits>

namespace terraformer::ui::main
{
	struct widget_geometry
	{
		location where;
		location origin;
		scaling size;

		[[nodiscard]] constexpr bool operator==(widget_geometry const&) const = default;
		[[nodiscard]] constexpr bool operator!=(widget_geometry const&) const = default;
	};

	[[nodiscard]] inline bool inside(wsapi::cursor_position pos, widget_geometry const& box)
	{
		auto const r = 0.5*box.size;
		auto const offset_to_origin = (location{0.0f, 0.0f, 0.0f} - box.origin).apply(r);
		auto const object_midpoint = box.where + offset_to_origin;
		auto const dr = location{static_cast<float>(pos.x), static_cast<float>(pos.y), 0.0f} - object_midpoint;
		return std::abs(dr[0]) < r[0] && std::abs(dr[1]) < r[1];
	}

	enum class widget_visibility:int{visible, not_rendered, collapsed};

	// Size options
	//
	// One of
	// * No constraint                                      <=> width = (0, inf),   height = (0, inf),   aspect ratio = nullopt
	// * Constraint in aspect ratio                         <=> width = (0, inf),   height = (0, inf),   aspect ratio = r
	// * Constraint in height (min, max)                    <=> width = (0, inf),   height = (min, max), aspect ratio = nullopt
	// * Constraint in height (min, max) + aspect ratio     <=> width = (0, inf),   height = (min, max), aspect ratio = r
	// * Constraint in width (min, max)                     <=> width = (min, max), height = (0, inf),   aspect ratio = nullopt
	// * Constraint in width (min, max) + aspect ratio      <=> width = (min, max), height = (0, inf),   aspect ratio = r
	// * Constraint in width (min, max) + height (min, max) <=> width = (min, max), height = (min, max), aspect ratio = nullopt
	// * Constraint in width (min, max) + height (min, max) <=> width = (min, max), height = (min, max), aspect ratio = r
	//   Requires at least one of
	//     w_min < h*r < w_max <=> w_min/r < h < w_max/r, that is [w_min/r, w_max/r] intersect [h_min, h_max] is non-empty
	//     h_min < w/r < h_max <=> h_min*r < w < h_max*r, that is [h_min*r, h_max*r] intersect [w_min, w_max] is non-empty
	//   If both fails, pick width and height in range to closest match the aspect ratio

	struct widget_size_range
	{
		float min = 0.0f;
		float max = std::numeric_limits<float>::infinity();
	};

	struct widget_size_constraint
	{
		widget_size_range width;
		widget_size_range height;
		std::optional<float> aspect_ratio;
	};


	template<class T, class TextureRepo, class OutputRectangle>
	concept widget = requires(
		T& obj,
		wsapi::fb_size size,
		wsapi::cursor_enter_leave_event const& cele,
		wsapi::cursor_motion_event const& cme,
		wsapi::mouse_button_event const& mbe,
		OutputRectangle& surface,
		TextureRepo const& textures,
		theming::widget_look const& look
	)
	{
		{ obj.render(surface, textures, look) } -> std::same_as<void>;
		{ obj.handle_event(cele) } -> std::same_as<void>;
		{ obj.handle_event(cme) } -> std::same_as<bool>;
		{ obj.handle_event(mbe) } -> std::same_as<bool>;
		{ obj.handle_event(std::as_const(size)) } -> std::same_as<wsapi::fb_size>;
	};

	struct widget_with_default_actions
	{
		template<class OutputRectangle, class TextureRepo>
		void render(OutputRectangle&&, TextureRepo&&, theming::widget_look const&) const {}
		void handle_event(wsapi::cursor_enter_leave_event const&);
		[[nodiscard]] bool handle_event(wsapi::cursor_motion_event const&) const { return false; }
		[[nodiscard]] bool handle_event(wsapi::mouse_button_event const&) const { return false; }
		[[nodiscard]] wsapi::fb_size handle_event(wsapi::fb_size size) const { return size; }
	};

	namespace
	{
		static_assert(widget<widget_with_default_actions, int, double>);
	}
}

#endif
