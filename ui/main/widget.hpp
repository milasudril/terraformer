#ifndef TERRAFORMER_UI_MAIN_WIDGET_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_HPP

#include "./events.hpp"
#include "./widget_rendering_result.hpp"

#include "ui/theming/widget_look.hpp"
#include "lib/common/object_tree.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/utils.hpp"
#include "lib/array_classes/multi_span.hpp"


#include <utility>
#include <type_traits>
#include <optional>

namespace terraformer::ui::main
{
	struct widget_instance_info
	{
		size_t section_level;
		size_t paragraph_index;
	};

	struct widget_geometry
	{
		location where;
		location origin;
		scaling size;

		[[nodiscard]] constexpr bool operator==(widget_geometry const&) const = default;
		[[nodiscard]] constexpr bool operator!=(widget_geometry const&) const = default;
	};

	[[nodiscard]] inline bool inside(cursor_position pos, widget_geometry const& box)
	{
		auto const r = 0.5*box.size;
		auto const offset_to_origin = (location{0.0f, 0.0f, 0.0f} - box.origin).apply(r);
		auto const object_midpoint = box.where + offset_to_origin;
		auto const dr = location{static_cast<float>(pos.x), static_cast<float>(pos.y), 0.0f} - object_midpoint;
		return std::abs(dr[0]) < r[0] && std::abs(dr[1]) < r[1];
	}

	enum class widget_visibility:int{visible, not_rendered, collapsed};

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

	template<class T>
	concept layout_policy = requires(T obj, widget_collection_view& widgets)
	{
		{std::as_const(obj).update_widget_locations(widgets)} -> std::same_as<widget_size_constraints>;
	};

	class layout_policy_ref
	{
	public:
		layout_policy_ref():
			m_update_widget_locations{
				[](void const*, widget_collection_view&){
					return widget_size_constraints{};
				}
			}
		{}

		template<layout_policy LayoutPolicy>
		explicit layout_policy_ref(std::reference_wrapper<LayoutPolicy> policy):
			m_handle{&policy.get()},
			m_update_widget_locations{
				[](void const* handle, widget_collection_view& widgets){
					return static_cast<LayoutPolicy*>(handle)->update_widget_locations(widgets);
				}
			}
		{}

		widget_size_constraints update_widget_locations(widget_collection_view& widgets) const
		{ return m_update_widget_locations(m_handle, widgets); }

	private:
		void const* m_handle;
		widget_size_constraints (*m_update_widget_locations)(void const*, widget_collection_view&);
	};

	using prepare_for_presentation_callback = void (*)(void*, widget_rendering_result);
	using cursor_enter_leave_callback = void (*)(void*, cursor_enter_leave_event const&);
	using cursor_position_callback = void (*)(void*, cursor_motion_event const&);
	using mouse_button_callback = void (*)(void*, mouse_button_event const&);
	using update_geometry_callback = widget_size_constraints (*)(void*);
	using size_callback = void (*)(void*, fb_size);
	using theme_updated_callback = void (*)(void*, object_dict const&);
	using get_children_callback = widget_collection_ref (*)(void*);
	using get_children_const_callback = widget_collection_view (*)(void const*);
	using get_layout_callback = layout_policy_ref (*)(void const*);

	template<bool IsConst>
	class widget_collection_ref_impl
	{
	public:
		using widget_span_raw = multi_span<
			void*,
			widget_visibility,
			widget_size_constraints,
			widget_geometry,
			prepare_for_presentation_callback,
			cursor_enter_leave_callback,
			cursor_position_callback,
			mouse_button_callback,
			update_geometry_callback,
			size_callback,
			theme_updated_callback,
			get_children_callback,
			get_children_const_callback,
			get_layout_callback
		>;

		using widget_span = std::conditional_t<IsConst,
			multi_span_const_t<widget_span_raw>,
			widget_span_raw
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
		{ return m_span.template get<0>(); }

		auto widget_visibilities() const
		{ return m_span.template get<1>(); }

		auto widget_visibilities()
		{ return m_span.template get<1>(); }

		auto size_constraints()
		{ return m_span.template get<2>(); }

		auto size_constraints() const
		{ return m_span.template get<2>(); }

		auto widget_geometries() const
		{ return m_span.template get<3>(); }

		auto widget_geometries()
		{ return m_span.template get<3>(); }

		auto render_callbacks() const
		{ return m_span.template get<4>(); }

		auto cursor_enter_leave_callbacks() const
		{ return m_span.template get<5>(); }

		auto cursor_motion_callbacks() const
		{ return m_span.template get<6>(); }

		auto mouse_button_callbacks() const
		{ return m_span.template get<7>(); }

		auto update_geometry_callbacks() const
		{ return m_span.template get<8>(); }

		auto size_callbacks() const
		{ return m_span.template get<9>(); }

		auto theme_updated_callbacks() const
		{ return m_span.template get<10>(); }

		auto get_children_callbacks()
		{ return m_span.template get<11>(); }

		auto get_children_const_callbacks() const
		{ return m_span.template get<12>(); }

		auto get_layout() const
		{ return m_span.template get<13>(); }

	private:
		widget_span m_span;
	};

	inline auto find(cursor_position pos, span<widget_geometry const> geoms)
	{
		return std::ranges::find_if(
			geoms,
			[pos](auto const& obj) {
				return inside(pos, obj);
			}
		);
	}

	struct widget_collection_ref : widget_collection_ref_impl<false>
	{ using widget_collection_ref_impl<false>::widget_collection_ref_impl; };

	struct widget_collection_view : widget_collection_ref_impl<true>
	{ using widget_collection_ref_impl<true>::widget_collection_ref_impl; };

	inline auto find(cursor_position pos, widget_collection_view const& widgets)
	{
		auto const i = find(pos, widgets.widget_geometries());
		if(i == std::end(widgets.widget_geometries()))
		{ return widget_collection_view::npos; }

		return widget_collection_view::index_type{
			static_cast<size_t>(i - std::begin(widgets.widget_geometries()))
		};
	}

	inline void theme_updated(widget_collection_view const& widgets, object_dict const& dict)
	{
		auto const theme_updated_callbacks = widgets.theme_updated_callbacks();
		auto const widget_pointers = widgets.widget_pointers();

		auto const n = std::size(widgets);
		for(auto k  = widgets.first_element_index(); k != n; ++k)
		{ theme_updated_callbacks[k](widget_pointers[k], dict); }
	}

	template<class T>
	concept widget = requires(
		T& obj,
		fb_size size,
		cursor_enter_leave_event const& cele,
		cursor_motion_event const& cme,
		mouse_button_event const& mbe,
		widget_instance_info const&,
		object_dict const& resources,
		widget_rendering_result surface
	)
	{
		{ obj.prepare_for_presentation(surface) } -> std::same_as<void>;
		{ obj.handle_event(cele) } -> std::same_as<void>;
		{ obj.handle_event(cme) } -> std::same_as<void>;
		{ obj.handle_event(mbe) } -> std::same_as<void>;
		{ obj.handle_event(std::as_const(size)) } -> std::same_as<void>;
		{ obj.update_geometry() } -> same_as_unqual<widget_size_constraints>;
		{ obj.theme_updated(resources) } -> std::same_as<void>;
		{ obj.get_children() } -> std::same_as<widget_collection_ref>;
		{ std::as_const(obj).get_children() } -> std::same_as<widget_collection_view>;
		{ std::as_const(obj).get_layout() } -> std::same_as<layout_policy_ref>;
	};

	struct widget_with_default_actions
	{
		void prepare_for_presentation(widget_rendering_result) const {}
		void handle_event(cursor_enter_leave_event const&);
		void handle_event(cursor_motion_event const&) const { }
		void handle_event(mouse_button_event const&) const { }
		void handle_event(fb_size) const { }

		[[nodiscard]] widget_size_constraints update_geometry() const
		{ return widget_size_constraints{}; }

		void theme_updated(object_dict const&) const {}

		[[nodiscard]] widget_collection_ref get_children()
		{ return widget_collection_ref{}; }

		[[nodiscard]] widget_collection_view get_children() const
		{ return widget_collection_view{}; }

		[[nodiscard]] layout_policy_ref get_layout() const
		{ return layout_policy_ref{}; }
	};

	namespace
	{
		static_assert(widget<widget_with_default_actions>);
	}
}

#endif
