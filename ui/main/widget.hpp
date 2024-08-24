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

	class widget_collection_view;

	using prepare_for_presentation_callback = void (*)(void*, widget_rendering_result);
	using cursor_enter_leave_callback = void (*)(void*, cursor_enter_leave_event const&);
	using cursor_position_callback = void (*)(void*, cursor_motion_event const&);
	using mouse_button_callback = void (*)(void*, mouse_button_event const&);
	using update_geometry_callback = widget_size_constraints (*)(void*);
	using size_callback = void (*)(void*, fb_size);
	using theme_updated_callback = void (*)(void*, object_dict const&);
	using get_childred_callback = widget_collection_view (*)(void*);

	class widget_collection_view
	{
	public:
		using widget_span = multi_span<
			void* const,
			widget_visibility const,
			widget_geometry const,
			prepare_for_presentation_callback const,
			cursor_enter_leave_callback const,
			cursor_position_callback const,
			mouse_button_callback const,
			update_geometry_callback const,
			size_callback const,
			theme_updated_callback const
		>;

		using index_type = typename widget_span::index_type;

		static constexpr index_type npos{static_cast<size_t>(-1)};

	private:

	};

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
	};

	namespace
	{
		static_assert(widget<widget_with_default_actions>);
	}
}

#endif
