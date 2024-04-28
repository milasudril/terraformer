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

	template<class T, class OutputRectangle, class TextureRepo>
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

	template<class OutputRectangle, class TextureRepo>
	struct widget_with_default_actions
	{
		using output_rectangle = OutputRectangle;
		using texture_repo = TextureRepo;
		void render(output_rectangle&, texture_repo const&, theming::widget_look const&) const {}
		void handle_event(wsapi::cursor_enter_leave_event const&);
		[[nodiscard]] bool handle_event(wsapi::cursor_motion_event const&) const { return false; }
		[[nodiscard]] bool handle_event(wsapi::mouse_button_event const&) const { return false; }
		[[nodiscard]] wsapi::fb_size handle_event(wsapi::fb_size size) const { return size; }
	};

	namespace
	{
		static_assert(widget<widget_with_default_actions<int, double>, int, double>);
	}
}

#endif
