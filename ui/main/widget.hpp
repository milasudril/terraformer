#ifndef TERRAFORMER_UI_MAIN_WIDGET_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_HPP

#include "./widget_collection_ref.hpp"

#include "lib/common/utils.hpp"
#include "lib/array_classes/multi_span.hpp"
#include "lib/common/value_accessor.hpp"
#include "lib/common/move_only_function.hpp"

#include <utility>
#include <type_traits>

namespace terraformer::ui::main
{
	template<class T>
	concept widget = requires(
		T& obj,
		fb_size size,
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
		{ obj.handle_event(std::as_const(size)) } -> std::same_as<void>;
		{ obj.theme_updated(cfg, instance_info) } -> std::same_as<void>;
		{ obj.get_children() } -> std::same_as<widget_collection_ref>;
		{ std::as_const(obj).get_children() } -> std::same_as<widget_collection_view>;
		{ obj.get_layout() } -> std::same_as<layout_policy_ref>;
		{ obj.compute_size(w_req) } -> std::same_as<scaling>;
		{ obj.compute_size(h_req) } -> std::same_as<scaling>;
	};

	class root_widget
	{
	public:
		explicit root_widget(
			widget_collection_ref const& widgets,
			widget_collection_ref::index_type index
		):
			m_widget{widgets.widget_pointers()[index]},
			m_children{widgets.get_children_callbacks()[index](m_widget)},
			m_compute_size_given_width{widgets.compute_size_given_width_callbacks()[index]},
			m_compute_size_given_height{widgets.compute_size_given_height_callbacks()[index]},
			m_size_confirmed{widgets.event_callbacks<fb_size>()[index]},
			m_old_size{
				.width = static_cast<int>(widgets.widget_geometries()[index].size[0]),
				.height = static_cast<int>(widgets.widget_geometries()[index].size[1])
			},
				m_layout{widgets.get_layout_callbacks()[index](m_widget)},
			m_prepare_for_presentation_callback{widgets.render_callbacks()[index]},
			m_layers{widgets.widget_layer_stacks()[index]},
			m_geometry{widgets.widget_geometries()[index]}
		{ }

		root_widget() = default;

		widget_collection_ref& children()
		{ return m_children; }

		scaling compute_size(widget_height_request req)
		{ return m_compute_size_given_width(m_widget, req); }

		scaling compute_size(widget_width_request req)
		{ return m_compute_size_given_height(m_widget, req); }

		void confirm_size(fb_size size)
		{
			if(m_old_size != size)
			{ m_size_confirmed(m_widget, size); }
		}

		scaling minimize_cell_sizes()
		{ return m_layout.minimize_cell_sizes(m_children); }

		void update_widget_locations()
		{ m_layout.update_widget_locations(m_children); }

		widget_layer_stack prepare_for_presentation(graphics_backend_ref backend)
		{ return m_prepare_for_presentation_callback(m_widget, backend); }

		template<class Renderer>
		void render(Renderer renderer, displacement offset)
		{
			value_of(renderer).render(m_geometry.where + offset, m_geometry.origin, m_geometry.size, m_layers);
		}

		widget_geometry const& geometry() const
		{ return m_geometry; }

	private:
		void* m_widget = nullptr;
		widget_collection_ref m_children;
		compute_size_given_width_callback m_compute_size_given_width = [](void*, widget_height_request){return scaling{};};
		compute_size_given_height_callback m_compute_size_given_height = [](void*, widget_width_request){return scaling{};};
		event_callback_t<fb_size> m_size_confirmed = [](void*, fb_size){};
		fb_size m_old_size{};
		layout_policy_ref m_layout;
		prepare_for_presentation_callback m_prepare_for_presentation_callback = [](void*, graphics_backend_ref){return widget_layer_stack{}; };
		widget_layer_stack m_layers{};
		widget_geometry m_geometry{};
	};

	inline scaling minimize_size(root_widget& root)
	{
		// TODO: Decide which dimension to minimize. Should be determined by parent
		auto const initial_size = root.compute_size(widget_width_request{});
		auto& children = root.children();
		auto const widget_states = children.widget_states();
		auto const sizes = children.sizes();
		for(auto k : children.element_indices())
		{
			if(!widget_states[k].collapsed) [[likely]]
			{
				root_widget next_root{children, k};
				sizes[k] = minimize_size(next_root);
			}
		}

		auto const size_from_layout = root.minimize_cell_sizes();

		return scaling{
			std::max(initial_size[0], size_from_layout[0]),
			std::max(initial_size[1], size_from_layout[1]),
			std::max(initial_size[2], size_from_layout[2])
		};
	}

	inline void update_widget_locations(root_widget& root)
	{
		root.update_widget_locations();
		auto& children = root.children();
		auto const widget_states = children.widget_states();
		for(auto k : children.element_indices())
		{
			if(!widget_states[k].collapsed) [[likely]]
			{
				root_widget next_root{children, k};
				update_widget_locations(next_root);
			}
		}
	}

	inline void confirm_sizes(root_widget& root, fb_size size)
	{
		root.confirm_size(size);
		auto children = root.children();
		auto const widget_states = children.widget_states();
		auto const widget_geometries = children.widget_geometries();
		auto const widget_sizes = children.sizes();
		for(auto k : children.element_indices())
		{
			if(!widget_states[k].collapsed) [[likely]]
			{
				root_widget next_root{children, k};
				confirm_sizes(
					next_root,
					fb_size{
						.width = static_cast<int>(widget_sizes[k][0]),
						.height = static_cast<int>(widget_sizes[k][1])
					}
				);
				widget_geometries[k].size = widget_sizes[k];
			}
		}
	}

	inline widget_layer_stack prepare_for_presentation(root_widget& root, graphics_backend_ref backend)
	{
		auto ret = root.prepare_for_presentation(backend);
		auto& children = root.children();
		auto const widget_states = children.widget_states();
		auto const layer_stacks = children.widget_layer_stacks();
		for(auto k : children.element_indices())
		{
			if(!widget_states[k].hidden) [[likely]]
			{
				root_widget next_root{children, k};
				layer_stacks[k] = prepare_for_presentation(next_root, backend);
			}
		}
		return ret;
	}

	template<class Renderer>
	inline void show_widgets(Renderer renderer, root_widget& root, displacement offset = displacement{0.0f, 0.0f, 0.0f})
	{
		root.render(renderer, offset);
		auto& children = root.children();
		auto const widget_states = children.widget_states();
		for(auto k : children.element_indices())
		{
			if(!widget_states[k].hidden) [[likely]]
			{
				root_widget next_root{children, k};
				show_widgets(renderer, next_root, root.geometry().where + offset - location{0.0f, 0.0f, 0.0f});
			}
		}
	}

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
		void handle_event(fb_size) {}

		void theme_updated(config const&, widget_instance_info) {}

		[[nodiscard]] widget_collection_ref get_children()
		{ return widget_collection_ref{}; }

		[[nodiscard]] widget_collection_view get_children() const
		{ return widget_collection_view{}; }

		[[nodiscard]] layout_policy_ref get_layout() const
		{ return layout_policy_ref{}; }

		[[nodiscard]] scaling compute_size(widget_width_request) const
		{ return scaling{}; }

		[[nodiscard]] scaling compute_size(widget_height_request) const
		{ return scaling{}; }
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
