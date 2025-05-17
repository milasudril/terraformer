#ifndef TERRAFORMER_UI_MAIN_WIDGET_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_HPP

#include "./widget_collection_ref.hpp"

#include "lib/common/utils.hpp"
#include "lib/array_classes/multi_span.hpp"
#include "lib/common/value_accessor.hpp"
#include "lib/common/move_only_function.hpp"
#include "lib/array_classes/single_array.hpp"

#include <utility>
#include <type_traits>

namespace terraformer::ui::main
{
	template<class T>
	concept widget = requires(
		T& obj,
		box_size size,
		cursor_enter_event const& cee,
		cursor_leave_event const& cle,
		cursor_motion_event const& cme,
		mouse_button_event const& mbe,
		keyboard_button_event const& kbe,
		widget_instance_info const&,
		config const& cfg,
		graphics_backend_ref backend,
		widget_instance_info instance_info,
		widget_width_request w_req,
		widget_height_request h_req,
		keyboard_focus_enter_event kfe,
		keyboard_focus_leave_event kfl,
		typing_event te,
		window_ref wr,
		ui_controller ui_ctrl
	)
	{
		{ obj.prepare_for_presentation(backend) } -> std::same_as<widget_layer_stack>;
		{ obj.handle_event(cee, wr, ui_ctrl) } -> std::same_as<void>;
		{ obj.handle_event(cle, wr, ui_ctrl) } -> std::same_as<void>;
		{ obj.handle_event(cme, wr, ui_ctrl) } -> std::same_as<void>;
		{ obj.handle_event(mbe, wr, ui_ctrl) } -> std::same_as<void>;
		{ obj.handle_event(kbe, wr, ui_ctrl) } -> std::same_as<void>;
		{ obj.handle_event(kfe, wr, ui_ctrl) } -> std::same_as<void>;
		{ obj.handle_event(kfl, wr, ui_ctrl) } -> std::same_as<void>;
		{ obj.handle_event(te, wr, ui_ctrl) } -> std::same_as<void>;
		{ obj.confirm_size(std::as_const(size)) } -> std::same_as<box_size>;
		{ obj.theme_updated(cfg, instance_info) } -> std::same_as<void>;
		{ obj.get_children() } -> std::same_as<widget_collection_ref>;
		{ std::as_const(obj).get_children() } -> std::same_as<widget_collection_view>;
		{ obj.get_layout() } -> std::same_as<layout_ref>;
		{ obj.compute_size(w_req) } -> std::same_as<box_size>;
		{ obj.compute_size(h_req) } -> std::same_as<box_size>;
	};

	template<class WidgetType>
	using widget_user_interaction_handler = move_only_function<void(WidgetType&, main::window_ref, main::ui_controller)>;

	struct widget_with_default_actions
	{
		widget_layer_stack prepare_for_presentation(graphics_backend_ref) { return widget_layer_stack{}; }
		void handle_event(cursor_enter_event const&, window_ref, ui_controller) {}
		void handle_event(cursor_leave_event const&, window_ref, ui_controller) {}
		void handle_event(cursor_motion_event const&, window_ref, ui_controller) {}
		void handle_event(mouse_button_event const&, window_ref, ui_controller) {}
		void handle_event(keyboard_button_event const&, window_ref, ui_controller) {}
		void handle_event(keyboard_focus_enter_event, window_ref, ui_controller){}
		void handle_event(keyboard_focus_leave_event, window_ref, ui_controller){}
		void handle_event(typing_event, window_ref, ui_controller){}

		void theme_updated(config const&, widget_instance_info) {}

		box_size confirm_size(box_size size) { return size; }

		[[nodiscard]] widget_collection_ref get_children()
		{ return widget_collection_ref{}; }

		[[nodiscard]] widget_collection_view get_children() const
		{ return widget_collection_view{}; }

		[[nodiscard]] layout_ref get_layout() const
		{ return layout_ref{}; }

		[[nodiscard]] box_size compute_size(widget_width_request) const
		{ return box_size{}; }

		[[nodiscard]] box_size compute_size(widget_height_request) const
		{ return box_size{}; }
	};

	static_assert(widget<widget_with_default_actions>);

	template<widget Widget>
	auto make_default_widget_state()
	{
		return widget_state{
			.collapsed = false,
			.hidden = false,
			.maximized = false,
			.disabled = false,
			.mbe_sensitive = !compare_with_fallback(
				resolve_overload<mouse_button_event const&>(&widget_with_default_actions::handle_event),
				resolve_overload<mouse_button_event const&>(&Widget::handle_event)
			),
			.kbe_sensitive = !(
				compare_with_fallback(
					resolve_overload<keyboard_button_event const&>(&widget_with_default_actions::handle_event),
					resolve_overload<keyboard_button_event const&>(&Widget::handle_event)
				)
				&& compare_with_fallback(
					resolve_overload<typing_event>(&widget_with_default_actions::handle_event),
					resolve_overload<typing_event>(&Widget::handle_event)
				)
			),
			.cursor_focus_indicator_mode = focus_indicator_mode::automatic,
			.kbd_focus_indicator_mode = focus_indicator_mode::automatic
		};
	}

	enum class widget_orientation{horizontal, vertical};
}

#endif
