#ifndef TERRAFORMER_UI_MAIN_WIDGET_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_HPP

#include "./events.hpp"

#include "ui/theming/widget_look.hpp"
#include "lib/common/object_tree.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/utils.hpp"
#include "lib/common/bitmask_enum.hpp"
#include "lib/any/unique_any.hpp"

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

	class input_device_grab;

	template<class T>
	concept input_event_sink = requires(
		T& obj,
		cursor_motion_event const& cme,
		mouse_button_event const& mbe,
		typing_event const& te,
		keyboard_button_event const& kbe,
		input_device_grab& current_grab
	)
	{
		{ obj.handle_event(cme, current_grab) } -> std::same_as<void>;
		{ obj.handle_event(mbe, current_grab) } -> std::same_as<void>;
		{ obj.handle_event(te, current_grab) } -> std::same_as<void>;
		{ obj.handle_event(kbe, current_grab) } -> std::same_as<void>;
	};

	template<class T>
	using event_callback = void (*)(void*, T const&, input_device_grab&);

	using cursor_motion_event_callback = event_callback<cursor_motion_event>;
	using mouse_button_event_callback = event_callback<mouse_button_event>;
	using typing_event_callback = event_callback<typing_event>;
	using keyboard_button_event_callback = event_callback<keyboard_button_event>;

	enum class input_device_mask:unsigned int {
		none = 0x0,
		default_keyboard = 0x1,
		default_mouse = 0x2
	};

	consteval void enable_bitmask_operators(input_device_mask){}

	class input_device_grab
	{
	public:
		struct widget_vtable
		{
			cursor_motion_event_callback on_cursor_moved;
			mouse_button_event_callback on_mouse_button_activated;
			typing_event_callback on_typing;
			keyboard_button_event_callback on_keyboard_button_activated;
		};

		template<class T>
		requires input_event_sink<T>
		static constexpr auto make_widget_vtable()
		{
			return widget_vtable{
				.on_cursor_moved = [](void* widget_ptr, cursor_motion_event const& event, input_device_grab& grab) {
					return static_cast<T*>(widget_ptr)->handle_event(event, grab);
				},
				.on_mouse_button_activated = [](void* widget_ptr, mouse_button_event const& event, input_device_grab& grab) {
					return static_cast<T*>(widget_ptr)->handle_event(event, grab);
				},
				.on_typing = [](void* widget_ptr, typing_event const& event, input_device_grab& grab) {
					return static_cast<T*>(widget_ptr)->handle_event(event, grab);
				},
				.on_keyboard_button_activated = [](void* widget_ptr, keyboard_button_event const& event, input_device_grab& grab){
					return static_cast<T*>(widget_ptr)->handle_event(event, grab);
				}
			};
		}

		template<class T>
		static constexpr widget_vtable vt = make_widget_vtable<T>();

		input_device_grab() = default;

		template<class T>
		requires input_event_sink<T>
		explicit input_device_grab(T& widget, input_device_mask grab_devices):
			m_widget_pointer{&widget},
			m_vtable{&vt<T>},
			m_active_devices{grab_devices}
		{}

		auto handle_event(cursor_motion_event const& event)
		{ return m_vtable->on_cursor_moved(m_widget_pointer, event, *this); }

		auto handle_event(mouse_button_event const& event)
		{ return m_vtable->on_mouse_button_activated(m_widget_pointer, event, *this); }

		auto handle_event(typing_event const& event)
		{ return m_vtable->on_typing(m_widget_pointer, event, *this); }

		auto handle_event(keyboard_button_event const& event)
		{ return m_vtable->on_keyboard_button_activated(m_widget_pointer, event, *this); }

		bool has_device(input_device_mask device) const
		{ return static_cast<bool>(m_active_devices & device); }

	private:
		void* m_widget_pointer = nullptr;
		widget_vtable const* m_vtable = nullptr;
		input_device_mask m_active_devices = input_device_mask::none;
	};

	template<class T, class ... OutputRectangle>
	concept widget = input_event_sink<T> && requires(
		T& obj,
		fb_size size,
		cursor_enter_leave_event const& cele,
		widget_instance_info const& instance_info,
		object_dict const& resources,
		OutputRectangle&... surface
	)
	{
		{ (..., obj.prepare_for_presentation(surface, instance_info, resources)) } -> std::same_as<void>;
		{ obj.handle_event(cele) } -> std::same_as<void>;
		{ obj.handle_event(std::as_const(size)) } -> std::same_as<void>;
		{ obj.get_size_constraints() } -> same_as_unqual<widget_size_constraints>;
		{ obj.theme_updated(resources) } -> std::same_as<void>;
	};


	struct widget_with_default_actions
	{
		template<class OutputRectangle>
		void prepare_for_presentation(OutputRectangle&&, widget_instance_info const&, object_dict const&) const {}
		void handle_event(cursor_enter_leave_event const&);
		void handle_event(cursor_motion_event const&, input_device_grab&) const { }
		void handle_event(mouse_button_event const&, input_device_grab&) const { }
		void handle_event(typing_event const&, input_device_grab&) const { }
		void handle_event(keyboard_button_event const&, input_device_grab&) const { }

		void handle_event(fb_size) const { }
		[[nodiscard]] widget_size_constraints get_size_constraints() const
		{ return widget_size_constraints{}; }
		void theme_updated(object_dict const&) const {}
	};

	namespace
	{
		static_assert(widget<widget_with_default_actions, double>);
	}
}

#endif
