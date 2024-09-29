#ifndef TERRAFORMER_UI_MAIN_WIDGET_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_HPP

#include "./widget_geometry.hpp"
#include "./widget_rendering_result.hpp"
#include "./config.hpp"

#include "lib/common/utils.hpp"
#include "lib/array_classes/multi_span.hpp"

#include <utility>
#include <type_traits>
#include <optional>

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

		bool has_cursor_focus_indicator() const
		{
			return (cursor_focus_indicator_mode == focus_indicator_mode::automatic && mbe_sensitive)
				|| cursor_focus_indicator_mode == focus_indicator_mode::always_visible;
		}

		bool has_keyboard_focus_indicator() const
		{
			return (kbd_focus_indicator_mode == focus_indicator_mode::automatic && kbe_sensitive)
				|| kbd_focus_indicator_mode == focus_indicator_mode::always_visible;
		}
	};

	struct widget_size_range
	{
		float min = 0.0f;
		float max = std::numeric_limits<float>::infinity();
	};

	struct widget_size_constraints
	{
		widget_size_range width;
		widget_size_range height;
		std::optional<float> aspect_ratio;
	};

	constexpr widget_size_constraints max(widget_size_constraints const& a, widget_size_constraints const& b)
	{
		auto const w_min = std::max(a.width.min, b.width.min);
		auto const w_max_temp = std::min(a.width.max, b.width.max);
		auto const w_max = w_max_temp < w_min ? w_min : w_max_temp;

		auto const h_min = std::max(a.height.min, b.height.min);
		auto const h_max_temp = std::min(a.height.max, b.height.max);
		auto const h_max = h_max_temp < h_min ? h_min : h_max_temp;

		std::optional<float> aspect_ratio{};
		if(a.aspect_ratio.has_value() || b.aspect_ratio.has_value())
		{
			auto const n = static_cast<int>(a.aspect_ratio.has_value()) + static_cast<int>(b.aspect_ratio.has_value());
			aspect_ratio = (n == 1)?
					a.aspect_ratio.value_or(1.0) * b.aspect_ratio.value_or(1.0f):
					std::sqrt((*a.aspect_ratio) * (*b.aspect_ratio));
		}

		return widget_size_constraints{
			.width{
				.min = w_min,
				.max = w_max
			},
			.height{
				.min = h_min,
				.max = h_max
			},
			.aspect_ratio = aspect_ratio
		};
	}

	inline scaling minimize_height(widget_size_constraints const& constraints)
	{
		auto const preliminary_height = constraints.height.min;
		if(constraints.aspect_ratio.has_value())
		{
			auto const width = std::clamp(
				*constraints.aspect_ratio*preliminary_height,
				constraints.width.min,
				constraints.width.max
			);

			auto const new_height = width/(*constraints.aspect_ratio);
			if(new_height < constraints.height.min || new_height > constraints.height.max)
			{ throw std::runtime_error{"Impossible size constraint"}; }

			return scaling{width, new_height, 1.0f};
		}

		return scaling{constraints.width.min, preliminary_height, 1.0f};
	};

	inline scaling minimize_width(widget_size_constraints const& constraints)
	{
		auto const preliminary_width = constraints.width.min;
		if(constraints.aspect_ratio.has_value())
		{
			auto const height = std::clamp(
				preliminary_width/(*constraints.aspect_ratio),
				constraints.height.min,
				constraints.height.max
			);

			auto const new_width = height*(*constraints.aspect_ratio);

			if(new_width < constraints.width.min || new_width > constraints.width.max)
			{ throw std::runtime_error{"Impossible size constraint"}; }

			return scaling{new_width, height, 1.0f};
		}

		return scaling{preliminary_width, constraints.height.min, 1.0f};
	}

	struct widget_collection_ref;
	struct widget_collection_view;

	struct widget_instance_info
	{
		size_t section_level;
		size_t paragraph_index;
	};

	template<class T>
	concept layout_policy = requires(T obj, widget_collection_ref& widgets)
	{
		{std::as_const(obj).update_widget_locations(widgets)} -> std::same_as<widget_size_constraints>;
	};

	class layout_policy_ref
	{
	public:
		layout_policy_ref():
			m_update_widget_locations{
				[](void const*, widget_collection_ref&){
					return widget_size_constraints{};
				}
			}
		{}

		template<layout_policy LayoutPolicy>
		explicit layout_policy_ref(std::reference_wrapper<LayoutPolicy> policy):
			m_handle{&policy.get()},
			m_update_widget_locations{
				[](void const* handle, widget_collection_ref& widgets){
					return static_cast<LayoutPolicy*>(handle)->update_widget_locations(widgets);
				}
			}
		{}

		widget_size_constraints update_widget_locations(widget_collection_ref& widgets) const
		{ return m_update_widget_locations(m_handle, widgets); }

	private:
		void const* m_handle;
		widget_size_constraints (*m_update_widget_locations)(void const*, widget_collection_ref&);
	};

	template<class EventType, class ... Args>
	using event_callback_t = void (*)(void*, EventType, Args...);

	using cursor_enter_callback = event_callback_t<cursor_enter_event const&>;
	using cursor_leave_callback = event_callback_t<cursor_leave_event const&>;
	using cursor_position_callback = event_callback_t<cursor_motion_event const&>;
	using mouse_button_callback = event_callback_t<mouse_button_event const&>;
	using size_callback = event_callback_t<fb_size>;
	using prepare_for_presentation_callback = event_callback_t<widget_rendering_result>;
	using theme_updated_callback = event_callback_t<config const&, widget_instance_info>;

	using compute_size_constraints_callback = widget_size_constraints (*)(void*);
	using get_children_callback = widget_collection_ref (*)(void*);
	using get_children_const_callback = widget_collection_view (*)(void const*);
	using get_layout_callback = layout_policy_ref (*)(void const*);

	template<bool IsConst>
	class widget_collection_ref_impl
	{
	public:
		using widget_span_mutable = multi_span<
			void*,
			widget_state,
			widget_size_constraints,
			widget_geometry,
			prepare_for_presentation_callback,
			cursor_enter_callback,
			cursor_leave_callback,
			cursor_position_callback,
			mouse_button_callback,
			compute_size_constraints_callback,
			size_callback,
			theme_updated_callback,
			get_children_callback,
			get_children_const_callback,
			get_layout_callback
		>;

		using widget_span_const = multi_span_const_t<widget_span_mutable>;

		using widget_span = std::conditional_t<IsConst,
			widget_span_const,
			widget_span_mutable
		>;

		using index_type = typename widget_span::index_type;

		static constexpr index_type npos{static_cast<size_t>(-1)};

		widget_collection_ref_impl() = default;

		explicit widget_collection_ref_impl(widget_span const& span): m_span{span}{}

		constexpr auto first_element_index() const
		{ return m_span.first_element_index(); }

		auto size() const
		{ return std::size(m_span); }

		auto widget_pointers() const
		{ return m_span.template get_by_type<void*>(); }

		auto widget_states() const
		{ return m_span.template get_by_type<widget_state>(); }

		template<class EventType>
		auto event_callbacks() const
		{
			if constexpr(m_span.template has_type<event_callback_t<EventType>>())
			{ return m_span.template get_by_type<event_callback_t<EventType>>(); }
			else
			{ return m_span.template get_by_type<event_callback_t<EventType const&>>(); }
		}

		auto size_constraints() const
		{ return m_span.template get_by_type<widget_size_constraints>(); }

		auto widget_geometries() const
		{ return m_span.template get_by_type<widget_geometry>(); }

		auto render_callbacks() const
		{ return m_span.template get_by_type<prepare_for_presentation_callback>(); }

		auto compute_size_constraints_callbacks() const
		{ return m_span.template get_by_type<compute_size_constraints_callback>(); }

		auto size_callbacks() const
		{ return m_span.template get_by_type<size_callback>(); }

		auto theme_updated_callbacks() const
		{ return m_span.template get_by_type<theme_updated_callback>(); }

		auto get_children_callbacks() const
		{ return m_span.template get_by_type<get_children_callback>(); }

		auto get_children_const_callbacks() const
		{ return m_span.template get_by_type<get_children_const_callback>(); }

		auto get_layout_callbacks() const
		{ return m_span.template get_by_type<get_layout_callback>(); }

		auto get_span() const
		{ return m_span; }

	private:
		widget_span m_span;
	};

	template<bool IsConstA, bool IsConstB>
	bool is_same(widget_collection_ref_impl<IsConstA> const& a,
		widget_collection_ref_impl<IsConstB> const& b)
	{ return std::data(a.widget_pointers()) == std::data(b.widget_pointers()); }

	inline auto find(cursor_position pos, span<widget_geometry const> geoms)
	{
		return std::ranges::find_if(
			geoms,
			[pos](auto const& obj) {
				return inside(pos, obj);
			}
		);
	}

	struct widget_collection_view : widget_collection_ref_impl<true>
	{ using widget_collection_ref_impl<true>::widget_collection_ref_impl; };

	struct widget_collection_ref : widget_collection_ref_impl<false>
	{
		using widget_collection_ref_impl<false>::widget_collection_ref_impl;

		widget_collection_view as_view() const
		{
			return widget_collection_view{widget_span_const{get_span()}};
		}
	};

	inline auto find(cursor_position pos, widget_collection_view const& widgets)
	{
		auto const i = find(pos, widgets.widget_geometries());
		if(i == std::end(widgets.widget_geometries())) [[likely]]
		{ return widget_collection_view::npos; }

		return widget_collection_view::index_type{
			static_cast<size_t>(i - std::begin(widgets.widget_geometries()))
		};
	}

	struct find_recursive_result
	{
		widget_collection_ref widget_collection{};
		widget_collection_view::index_type index{widget_collection_ref::npos};

		bool operator==(find_recursive_result const& other) const
		{ return is_same(widget_collection, other.widget_collection) && index == other.index; }

		bool operator!=(find_recursive_result const& other) const
		{ return !(*this == other); }

		[[nodiscard]] bool empty() const
		{ return index == widget_collection_ref::npos; }

		widget_geometry geometry() const
		{
			return widget_collection.widget_geometries()[index];
		}

		widget_state state() const
		{ return widget_collection.widget_states()[index]; }
	};

	template<class EventType>
	bool try_dispatch(EventType&& e, find_recursive_result const& res)
	{
		if(res.empty())
		{ return false; }

		auto const widgets = res.widget_collection.widget_pointers();
		auto const callbacks = res.widget_collection.template event_callbacks<EventType>();
		callbacks[res.index](widgets[res.index], std::forward<EventType>(e));

		return true;
	}

	inline auto find_recursive(cursor_position pos, widget_collection_ref const& widgets)
	{
		// Is pos within any widget at this level
		auto const i = find(pos, widgets.as_view());
		if(i == widget_collection_view::npos) [[likely]]
		{
			// No, return empty
			return find_recursive_result{widget_collection_ref{}, widget_collection_view::npos};
		}

		// Is pos within any child widget
		auto const widget_pointers = widgets.widget_pointers();
		auto const get_children_callbacks = widgets.get_children_callbacks();
		auto const j = find_recursive(pos, get_children_callbacks[i](widget_pointers[i]));
		if(j.index == widget_collection_view::npos) [[unlikely]]
		{
			// No, return the widget at current level
			return find_recursive_result{widgets, i};
		}

		// Yes, return the widget at child level
		return j;
	}

	inline void theme_updated(
		widget_collection_view const& widgets,
		config const& cfg,
		widget_instance_info instance_info = widget_instance_info{}
	)
	{
		auto const theme_updated_callbacks = widgets.theme_updated_callbacks();
		auto const widget_pointers = widgets.widget_pointers();
		auto const get_children_callbacks = widgets.get_children_const_callbacks();

		auto const n = std::size(widgets);
		for(auto k = widgets.first_element_index(); k != n; ++k)
		{
			theme_updated_callbacks[k](
				widget_pointers[k],
				cfg,
				widget_instance_info{
					.section_level = instance_info.section_level,
					.paragraph_index = k.get()
				}
			);
			auto const children = get_children_callbacks[k](widget_pointers[k]);
			theme_updated(
				children,
				cfg,
				widget_instance_info{
					.section_level = instance_info.section_level + 1,
					.paragraph_index = 0
				}
			);
		}
	}

	template<class T>
	concept widget = requires(
		T& obj,
		fb_size size,
		cursor_enter_event const& cee,
		cursor_leave_event const& cle,
		cursor_motion_event const& cme,
		mouse_button_event const& mbe,
		widget_instance_info const&,
		config const& cfg,
		widget_rendering_result surface,
		widget_instance_info instance_info
	)
	{
		{ obj.prepare_for_presentation(surface) } -> std::same_as<void>;
		{ obj.handle_event(cee) } -> std::same_as<void>;
		{ obj.handle_event(cle) } -> std::same_as<void>;
		{ obj.handle_event(cme) } -> std::same_as<void>;
		{ obj.handle_event(mbe) } -> std::same_as<void>;
		{ obj.handle_event(std::as_const(size)) } -> std::same_as<void>;
		{ obj.compute_size_constraints() } -> same_as_unqual<widget_size_constraints>;
		{ obj.theme_updated(cfg, instance_info) } -> std::same_as<void>;
		{ obj.get_children() } -> std::same_as<widget_collection_ref>;
		{ std::as_const(obj).get_children() } -> std::same_as<widget_collection_view>;
		{ std::as_const(obj).get_layout() } -> std::same_as<layout_policy_ref>;
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
			m_compute_size_contraints{widgets.compute_size_constraints_callbacks()[index]},
			m_size_confirmed{widgets.event_callbacks<fb_size>()[index]},
			m_layout{widgets.get_layout_callbacks()[index](m_widget)}
		{}

		root_widget() = default;

		template<widget Widget>
		explicit root_widget(std::reference_wrapper<Widget> w):
			m_widget{&w.get()},
			m_children{w.get().get_children()},
			m_compute_size_contraints{
				[](void* obj) {
					return static_cast<Widget*>(obj)->compute_size_constraints();
				}
			},
			m_size_confirmed{
				[](void* obj, fb_size size) {
					return static_cast<Widget*>(obj)->handle_event(size);
				}
			},
			m_layout{w.get().get_layout()}
		{}

		widget_collection_ref& children()
		{ return m_children; }

		widget_size_constraints compute_size_constraints()
		{ return m_compute_size_contraints(m_widget); }

		void confirm_size(fb_size size)
		{ m_size_confirmed(m_widget, size); }

		widget_size_constraints run_layout()
		{ return m_layout.update_widget_locations(m_children); }

	private:
		void* m_widget = nullptr;
		widget_collection_ref m_children;
		compute_size_constraints_callback m_compute_size_contraints = [](void*){return widget_size_constraints{};};
		event_callback_t<fb_size> m_size_confirmed =[](void*, fb_size){};
		layout_policy_ref m_layout;
	};

	inline widget_size_constraints compute_size_constraints(root_widget& root)
	{
		auto const initial_constriants = root.compute_size_constraints();
		auto& children = root.children();
		auto const widget_states = children.widget_states();
		auto const widget_size_constraints = children.size_constraints();
		auto const n = std::size(children);
		for(auto k = children.first_element_index(); k != n; ++k)
		{
			if(!widget_states[k].collapsed) [[likely]]
			{
				root_widget next_root{children, k};
				widget_size_constraints[k] = compute_size_constraints(next_root);
			}
		}

		auto const contraints_from_layout = root.run_layout();

		return max(initial_constriants, contraints_from_layout);
	}

	inline void confirm_sizes(root_widget& root, fb_size size)
	{
		root.confirm_size(size);
		auto children = root.children();
		auto const widget_states = children.widget_states();
		auto const widget_geometries = children.widget_geometries();
		auto const n = std::size(children);
		for(auto k = children.first_element_index(); k!=n; ++k)
		{
			if(!widget_states[k].collapsed) [[likely]]
			{
				root_widget next_root{children, k};
				confirm_sizes(
					next_root,
					fb_size{
						.width = static_cast<int>(widget_geometries[k].size[0]),
						.height = static_cast<int>(widget_geometries[k].size[1])
					}
				);
			}
		}
	}

	inline void apply_offsets(root_widget& root, displacement root_offset)
	{
		auto& children = root.children();
		auto const n = std::size(children);
		auto const widget_geometries = children.widget_geometries();
		for(auto k = children.first_element_index(); k != n; ++k)
		{ widget_geometries[k].where += root_offset; }

		for(auto k = children.first_element_index(); k != n; ++k)
		{
			root_widget next_root{children, k};
			apply_offsets(
				next_root,
				widget_geometries[k].where - location{0.0f, 0.0f, 0.0f}
			);
		}
	}

	struct widget_with_default_actions
	{
		void prepare_for_presentation(widget_rendering_result) {}
		void handle_event(cursor_enter_event const&) {}
		void handle_event(cursor_leave_event const&) {}
		void handle_event(cursor_motion_event const&) {}
		void handle_event(mouse_button_event const&) {}
		void handle_event(fb_size) {}

		[[nodiscard]] widget_size_constraints compute_size_constraints()
		{ return widget_size_constraints{}; }

		void theme_updated(config const&, widget_instance_info) {}

		[[nodiscard]] widget_collection_ref get_children()
		{ return widget_collection_ref{}; }

		[[nodiscard]] widget_collection_view get_children() const
		{ return widget_collection_view{}; }

		[[nodiscard]] layout_policy_ref get_layout() const
		{ return layout_policy_ref{}; }
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
			.kbe_sensitive = false,
			.cursor_focus_indicator_mode = focus_indicator_mode::automatic,
			.kbd_focus_indicator_mode = focus_indicator_mode::automatic
		};
	}
}

#endif
