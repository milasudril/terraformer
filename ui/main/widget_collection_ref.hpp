//@	{"dependencies_extra":[{"ref":"./widget_collection_ref.o", "rel": "implementation"}]}

#ifndef TERRAFORMER_UI_MAIN_WIDGET_COLLECTION_REF_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_COLLECTION_REF_HPP

#include "./events.hpp"
#include "./config.hpp"
#include "./widget_layer_stack.hpp"
#include "./widget_state.hpp"
#include "./widget_geometry.hpp"
#include "./ui_controller.hpp"
#include "./window_ref.hpp"
#include "./widget_layer_stack.hpp"
#include "./graphics_backend_ref.hpp"
#include "./layout.hpp"

#include "lib/common/value_accessor.hpp"


#include <concepts>
#include <utility>
#include <functional>
#include <optional>

namespace terraformer::ui::main
{
	struct widget_collection_ref;
	struct widget_collection_view;

	struct widget_instance_info
	{
		size_t section_level;
		size_t paragraph_index;
	};

	struct computed_width
	{ float value; };

	struct computed_height
	{ float value; };

	template<class T>
	concept layout_policy = requires(
		T obj,
		widget_collection_ref& widgets,
		widget_collection_view const& widgets_view,
		scaling available_size
	)
	{
		{obj.set_default_cell_sizes(widgets_view)} -> std::same_as<scaling>;
		{obj.adjust_cell_sizes(available_size)} -> std::same_as<scaling>;
		{std::as_const(obj).update_widget_locations(widgets)} -> std::same_as<void>;
		// TODO: Add set_cell_widths
	};

	class layout_policy_ref
	{
	public:
		layout_policy_ref():
			m_handle{nullptr},
			m_set_default_cell_sizes{
				[](void*, widget_collection_view const&){
					return scaling{};
				}
			},
			m_adjust_cell_sizes{
				[](void*, scaling available_size){
					return available_size;
				}
			},
			m_update_widget_locations{
				[](void const*, widget_collection_ref&){}
			}
		{}

		template<layout_policy LayoutPolicy>
		explicit layout_policy_ref(std::reference_wrapper<LayoutPolicy> policy):
			m_handle{&policy.get()},
			m_set_default_cell_sizes{
				[](void* handle, widget_collection_view const& widgets) {
					return static_cast<LayoutPolicy*>(handle)->set_default_cell_sizes(widgets);
				}
			},
			m_adjust_cell_sizes{
				[](void* handle, scaling available_size) {
					return static_cast<LayoutPolicy*>(handle)->adjust_cell_sizes(available_size);
				}
			},
			m_update_widget_locations{
				[](void const* handle, widget_collection_ref& widgets){
					static_cast<LayoutPolicy const*>(handle)->update_widget_locations(widgets);
				}
			}
#if 0
			m_set_cell_widths{
				[](void* handle, float available_width) {
					return static_cast<LayoutPolicy const*>(handle)->set_cell_widths(available_width);
				}
			}
#endif

		{}


		scaling set_default_cell_sizes(widget_collection_view const& widgets) const
		{ return m_set_default_cell_sizes(m_handle, widgets); }

		scaling adjust_cell_sizes(scaling available_size) const
		{ return m_adjust_cell_sizes(m_handle, available_size); }

		void update_widget_locations(widget_collection_ref& widgets) const
		{ m_update_widget_locations(m_handle, widgets); }

		bool is_valid() const
		{ return m_handle != nullptr; }

		terraformer::scaling set_cell_widths(float available_width) const
		{ return m_set_cell_widths(m_handle, available_width); }

	private:
		void* m_handle;
		scaling (*m_set_default_cell_sizes)(void*, widget_collection_view const&);
		scaling (*m_adjust_cell_sizes)(void*, scaling);
		void (*m_update_widget_locations)(void const*, widget_collection_ref&);
		scaling (*m_set_cell_widths)(void*, float);
	};

	template<class EventType, class ... Args>
	using event_callback_t = void (*)(void*, EventType, Args...);

	struct widget_width_request
	{ float height; };

	struct widget_height_request
	{ float width; };

	using cursor_enter_callback = event_callback_t<cursor_enter_event const&, window_ref, ui_controller>;
	using cursor_leave_callback = event_callback_t<cursor_leave_event const&, window_ref, ui_controller>;
	using cursor_position_callback = event_callback_t<cursor_motion_event const&, window_ref, ui_controller>;
	using mouse_button_callback = event_callback_t<mouse_button_event const&, window_ref, ui_controller>;
	using keyboard_button_callback = event_callback_t<keyboard_button_event const&, window_ref, ui_controller>;
	using typing_callback = event_callback_t<typing_event, window_ref, ui_controller>;
	using keyboard_focus_enter_callback = event_callback_t<keyboard_focus_enter_event, window_ref, ui_controller>;
	using keyboard_focus_leave_callback = event_callback_t<keyboard_focus_leave_event, window_ref, ui_controller>;
	using size_callback = event_callback_t<fb_size>;
	using prepare_for_presentation_callback = widget_layer_stack (*)(void*, graphics_backend_ref);
	using theme_updated_callback = event_callback_t<config const&, widget_instance_info>;

	using compute_size_given_height_callback = scaling (*)(void*, widget_width_request);
	using compute_size_given_width_callback = scaling (*)(void*, widget_height_request);
	using get_children_callback = widget_collection_ref (*)(void*);
	using get_children_const_callback = widget_collection_view (*)(void const*);
	using get_layout_callback = layout_ref (*)(void*);

	template<bool IsConst>
	class widget_collection_ref_impl
	{
	public:
		using widget_span_mutable = multi_span<
			void*,
			widget_state,
			scaling,
			computed_width,
			computed_height,
			widget_geometry,
			widget_layer_stack,
			prepare_for_presentation_callback,
			cursor_enter_callback,
			cursor_leave_callback,
			cursor_position_callback,
			mouse_button_callback,
			keyboard_button_callback,
			typing_callback,
			keyboard_focus_enter_callback,
			keyboard_focus_leave_callback,
			compute_size_given_height_callback,
			compute_size_given_width_callback,
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

		using reference = typename widget_span::reference;

		static constexpr index_type npos{static_cast<size_t>(-1)};

		widget_collection_ref_impl() = default;

		explicit widget_collection_ref_impl(widget_span const& span): m_span{span}{}

		auto element_indices() const
		{ return m_span.element_indices(); }

		auto size() const
		{ return std::size(m_span); }

		auto widget_pointers() const
		{ return m_span.template get_by_type<void*>(); }

		auto widget_states() const
		{ return m_span.template get_by_type<widget_state>(); }

		auto widget_layer_stacks() const
		{ return m_span.template get_by_type<widget_layer_stack>(); }

		template<class EventType, class ... Args>
		auto event_callbacks() const
		{
			if constexpr(m_span.template has_type<event_callback_t<EventType, Args...>>())
			{ return m_span.template get_by_type<event_callback_t<EventType, Args...>>(); }
			else
			{ return m_span.template get_by_type<event_callback_t<EventType const&, Args...>>(); }
		}

		auto sizes() const
		{ return m_span.template get_by_type<scaling>(); }

		auto computed_widths() const
		{ return m_span.template get_by_type<computed_width>(); }

		auto computed_heights() const
		{ return m_span.template get_by_type<computed_height>(); }

		auto widget_geometries() const
		{ return m_span.template get_by_type<widget_geometry>(); }

		auto render_callbacks() const
		{ return m_span.template get_by_type<prepare_for_presentation_callback>(); }

		auto compute_size_given_height_callbacks() const
		{ return m_span.template get_by_type<compute_size_given_height_callback>(); }

		auto compute_size_given_width_callbacks() const
		{ return m_span.template get_by_type<compute_size_given_width_callback>(); }

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

		auto extract(index_type k) const
		{ return m_span[k]; }

	private:
		widget_span m_span;
	};

	template<bool IsConstA, bool IsConstB>
	bool is_same(widget_collection_ref_impl<IsConstA> const& a,
		widget_collection_ref_impl<IsConstB> const& b)
	{ return std::data(a.widget_pointers()) == std::data(b.widget_pointers()); }

	inline auto find(cursor_position pos, span<widget_geometry const> geoms, displacement offset)
	{
		return std::ranges::find_if(
			geoms,
			[pos = location{static_cast<float>(pos.x), static_cast<float>(pos.y), 0.0f} - offset](auto const& obj) {
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

	class minimize_cell_size_context
	{
	public:
		explicit minimize_cell_size_context(
			widget_collection_ref const& widgets,
			widget_collection_ref::index_type index
		):
			m_widget{widgets.widget_pointers()[index]},
			m_compute_size_given_height{widgets.compute_size_given_height_callbacks()[index]},
			m_children{widgets.get_children_callbacks()[index](m_widget)},
			m_layout{widgets.get_layout_callbacks()[index](m_widget)}
		{ }

		scaling compute_size(widget_width_request req) const
		{ return m_compute_size_given_height(m_widget, req); }

		widget_collection_ref const& children() const
		{ return m_children; }

		layout_ref get_layout() const
		{ return m_layout; }

	private:
		void* m_widget;
		scaling (*m_compute_size_given_height)(void*, widget_width_request);
		widget_collection_ref m_children;
		layout_ref m_layout;
	};

	scaling run(minimize_cell_size_context const& ctxt);

	class confirm_widget_size_context
	{
	public:
		explicit confirm_widget_size_context(
			widget_collection_ref const& widgets,
			widget_collection_ref::index_type index
		):
			m_widget{widgets.widget_pointers()[index]},
			m_old_size{
				.width = static_cast<int>(widgets.widget_geometries()[index].size[0]),
				.height = static_cast<int>(widgets.widget_geometries()[index].size[1])
			},
			m_size_confirmed{widgets.event_callbacks<fb_size>()[index]},
			m_children{widgets.get_children_callbacks()[index](m_widget)}
		{ }

		widget_collection_ref const& children() const
		{ return m_children; }

		void confirm_size(fb_size size) const
		{
			if(size != m_old_size)
			{ m_size_confirmed(m_widget, size); }
		}

	private:
		void* m_widget;
		fb_size m_old_size;
		event_callback_t<fb_size> m_size_confirmed = [](void*, fb_size){};
		widget_collection_ref m_children;
	};

	void run(confirm_widget_size_context const& ctxt, fb_size size);

	class update_widget_location_context
	{
	public:
		explicit update_widget_location_context(
			widget_collection_ref const& widgets,
			widget_collection_ref::index_type index
		):
			m_widget{widgets.widget_pointers()[index]},
			m_children{widgets.get_children_callbacks()[index](m_widget)},
			m_layout{widgets.get_layout_callbacks()[index](m_widget)}
		{ }

		widget_collection_ref const& children() const
		{ return m_children; }

		layout_ref get_layout() const
		{ return m_layout; }

	private:
		void* m_widget;
		widget_collection_ref m_children;
		layout_ref m_layout;
	};

	void run(update_widget_location_context const& ctxt);

	class prepare_for_presentation_context
	{
	public:
		explicit prepare_for_presentation_context(
			widget_collection_ref const& widgets,
			widget_collection_ref::index_type index
		):
			m_widget{widgets.widget_pointers()[index]},
			m_old_size{
				.width = static_cast<int>(widgets.widget_geometries()[index].size[0]),
				.height = static_cast<int>(widgets.widget_geometries()[index].size[1])
			},
			m_prepare_for_presentation_callback{widgets.render_callbacks()[index]},
			m_children{widgets.get_children_callbacks()[index](m_widget)}
		{ }

		widget_collection_ref const& children() const
		{ return m_children; }

		widget_layer_stack prepare_for_presentation(graphics_backend_ref backend) const
		{ return m_prepare_for_presentation_callback(m_widget, backend); }

	private:
		void* m_widget;
		fb_size m_old_size;
		prepare_for_presentation_callback m_prepare_for_presentation_callback = [](void*, graphics_backend_ref){return widget_layer_stack{}; };
		widget_collection_ref m_children;
	};

	widget_layer_stack run(prepare_for_presentation_context const& ctxt, graphics_backend_ref backend);

	class show_widget_context
	{
	public:
		explicit show_widget_context(
			widget_collection_ref const& widgets,
			widget_collection_ref::index_type index
		):
			m_widget{widgets.widget_pointers()[index]},
			m_geometry{widgets.widget_geometries()[index]},
			m_layers{widgets.widget_layer_stacks()[index]},
			m_children{widgets.get_children_callbacks()[index](m_widget)}
		{ }

		widget_collection_ref const& children() const
		{ return m_children; }

		template<class Renderer>
		void render(Renderer renderer, displacement offset) const
		{ value_of(renderer).render(m_geometry.where + offset, m_geometry.origin, m_geometry.size, m_layers); }

		widget_geometry const& geometry() const
		{ return m_geometry; }

	private:
		void* m_widget;
		widget_geometry m_geometry{};
		widget_layer_stack m_layers{};
		widget_collection_ref m_children;
	};

	template<class Renderer>
	void run(
		show_widget_context const& ctxt,
		Renderer renderer,
		displacement offset = displacement{0.0f, 0.0f, 0.0f}
	)
	{
		ctxt.render(renderer, offset);
		auto& children = ctxt.children();
		auto const widget_states = children.widget_states();
		for(auto k : children.element_indices())
		{
			if(!widget_states[k].hidden) [[likely]]
			{
				run(
					show_widget_context{children, k},
					renderer,
					ctxt.geometry().where + offset - location{0.0f, 0.0f, 0.0f}
				);
			}
		}
	}

	template<class RequestType>
	struct set_default_cell_size
	{
		void* current_widget;
		RequestType new_size;
		scaling (*compute_size)(void*, RequestType);
		widget_collection_ref children;
		layout_policy_ref current_layout;
	};

	using set_cell_width_context = set_default_cell_size<widget_height_request>;

	scaling run(set_cell_width_context const& ctxt);

	using set_cell_height_context = set_default_cell_size<widget_width_request>;

	scaling run(set_cell_height_context const& ctxt);

	struct adjust_cell_sizes_context
	{
		void* current_widget;
		widget_collection_ref children;
		layout_policy_ref current_layout;
	};

	scaling run(adjust_cell_sizes_context const& ctxt, scaling available_size);

	struct set_cell_widths_context
	{
		void* current_widget;
		widget_collection_ref children;
		layout_policy_ref current_layout;
	};

	scaling run(set_cell_widths_context const& ctxt, float available_width);

	struct set_cell_heights_context
	{
		void* current_widget;
		widget_collection_ref children;
		layout_policy_ref current_layout;
	};

	scaling run(set_cell_heights_context const& ctxt, float available_height);


	inline auto find(cursor_position pos, widget_collection_view const& widgets, displacement offset)
	{
		auto const i = find(pos, widgets.widget_geometries(), offset);
		if(i == std::end(widgets.widget_geometries())) [[likely]]
		{ return widget_collection_view::npos; }

		return widgets.element_indices().front() + (i - std::begin(widgets.widget_geometries()));
	}

	class find_recursive_result
	{
	public:
		find_recursive_result() = default;

		explicit find_recursive_result(
			widget_collection_ref const& widgets,
			widget_collection_ref::index_type index,
			displacement offset
		):m_widgets{widgets},
			m_index{index},
			m_offset{offset}
		{}

		bool operator==(find_recursive_result const& other) const
		{ return is_same(m_widgets, other.m_widgets) && m_index == other.m_index; }

		bool operator!=(find_recursive_result const& other) const
		{ return !(*this == other); }

		[[nodiscard]] bool empty() const
		{ return m_index == widget_collection_ref::npos; }

		widget_geometry geometry() const
		{ return m_widgets.widget_geometries()[m_index]; }

		auto pointer() const
		{ return m_widgets.widget_pointers()[m_index]; }

		widget_state state() const
		{ return m_widgets.widget_states()[m_index]; }

		auto widgets() const
		{ return m_widgets; }

		auto index() const
		{ return m_index; }

		auto geometric_offset() const
		{ return m_offset; }

	private:
		widget_collection_ref m_widgets{};
		widget_collection_ref::index_type m_index{widget_collection_ref::npos};
		displacement m_offset;
	};

	template<class EventType, class ... Args>
	bool try_dispatch(EventType&& e, find_recursive_result const& res, Args&&... args)
	{
		if(res.empty())
		{ return false; }

		auto const widgets = res.widgets().widget_pointers();
		auto const callbacks = res.widgets().template event_callbacks<std::remove_cvref_t<EventType>, std::remove_cvref_t<Args>...>();
		if constexpr(requires{e.where;})
		{
			auto const geoms = res.widgets().widget_geometries();
			auto const offset = res.geometric_offset();
			e.where.x -= geoms[res.index()].where[0] + offset[0];
			e.where.y -= geoms[res.index()].where[1] + offset[1];
		}
		callbacks[res.index()](widgets[res.index()], std::forward<EventType>(e), std::forward<Args>(args)...);

		return true;
	}

	find_recursive_result find_recursive(
		cursor_position pos,
		widget_collection_ref const& widgets,
		displacement offset = displacement{0.0f, 0.0f, 0.0f}
	);

	void theme_updated(
		widget_collection_view const& widgets,
		config const& cfg,
		widget_instance_info instance_info = widget_instance_info{}
	);
}
#endif
