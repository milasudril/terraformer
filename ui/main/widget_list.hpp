#ifndef TERRAFORMER_UI_MAIN_WIDGET_LIST_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_LIST_HPP

#include "./widget.hpp"
#include "lib/array_classes/multi_array.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"

namespace terraformer::ui::main
{
	template<class WidgetRenderingResult>
	using prepare_for_presentation_callback = void (*)(void*, WidgetRenderingResult&);

	template<class WidgetRenderingResult>
	class widget_list
	{
	public:
		using cursor_enter_leave_callback = void (*)(void*, cursor_enter_leave_event const&);
		using cursor_position_callback = void (*)(void*, cursor_motion_event const&);
		using mouse_button_callback = void (*)(void*, mouse_button_event const&);
		using update_geometry_callback = widget_size_constraints (*)(void*);
		using size_callback = void (*)(void*, fb_size);
		using theme_updated_callback = void (*)(void*, object_dict const&);

		using widget_array = multi_array<
			void*,
			WidgetRenderingResult,
			widget_visibility,
			widget_geometry,
			prepare_for_presentation_callback<WidgetRenderingResult>,
			cursor_enter_leave_callback,
			cursor_position_callback,
			mouse_button_callback,
			update_geometry_callback,
			size_callback,
			theme_updated_callback
		>;

		using index_type = typename widget_array::index_type;

		static constexpr index_type npos{static_cast<size_t>(-1)};

		template<class Widget>
		requires widget<Widget, WidgetRenderingResult>
		widget_list& append(
			std::reference_wrapper<Widget> w,
			widget_geometry const& initial_geometry,
			widget_visibility initial_visibility = widget_visibility::visible
		)
		{
			m_objects.push_back(
				&w.get(),
				WidgetRenderingResult{},
				initial_visibility,
				initial_geometry,
				[](void* obj, WidgetRenderingResult& result) -> void {
					return static_cast<Widget*>(obj)->prepare_for_presentation(result);
				},
				[](void* obj, cursor_enter_leave_event const& event) -> void{
					static_cast<Widget*>(obj)->handle_event(event);
				},
				[](void* obj, cursor_motion_event const& event) -> void{
					static_cast<Widget*>(obj)->handle_event(event);
				},
				[](void* obj, mouse_button_event const& mbe) -> void {
					static_cast<Widget*>(obj)->handle_event(mbe);
				},
				[](void* obj) -> widget_size_constraints {
					return static_cast<Widget*>(obj)->update_geometry();
				},
				[](void* obj, fb_size size) {
					static_cast<Widget*>(obj)->handle_event(size);
				},
				[](void* obj, object_dict const& new_theme) {
					static_cast<Widget*>(obj)->theme_updated(new_theme);
				}
			);

			return *this;
		}

		constexpr auto first_element_index() const
		{ return m_objects.first_element_index(); }

		auto size() const
		{ return std::size(m_objects); }

		auto widget_pointers() const
		{ return m_objects.template get<0>(); }

		auto output_rectangles() const
		{ return m_objects.template get<1>(); }

		auto output_rectangles()
		{ return m_objects.template get<1>(); }

		auto widget_visibilities() const
		{ return m_objects.template get<2>(); }

		auto widget_visibilities()
		{ return m_objects.template get<2>(); }

		auto widget_geometries() const
		{ return m_objects.template get<3>(); }

		auto widget_geometries()
		{ return m_objects.template get<3>(); }

		auto render_callbacks() const
		{ return m_objects.template get<4>(); }

		auto cursor_enter_leave_callbacks() const
		{ return m_objects.template get<5>(); }

		auto cursor_motion_callbacks() const
		{ return m_objects.template get<6>(); }

		auto mouse_button_callbacks() const
		{ return m_objects.template get<7>(); }

		auto update_geometry_callbacks() const
		{ return m_objects.template get<8>(); }

		auto size_callbacks() const
		{ return m_objects.template get<9>(); }

		auto const theme_updated_callbacks() const
		{ return m_objects.template get<10>(); }

	private:
		widget_array m_objects;
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

	template<class ... WidgetRenderingResult>
	auto find(cursor_position pos, widget_list<WidgetRenderingResult...> const& widgets)
	{
		using wl = widget_list<WidgetRenderingResult...>;
		auto const i = find(pos, widgets.widget_geometries());
		if(i == std::end(widgets.widget_geometries()))
		{ return wl::npos; }

		return typename wl::index_type{
			static_cast<size_t>(i - std::begin(widgets.widget_geometries()))
		};
	}

	template<class ... WidgetRenderingResult>
	void theme_updated(widget_list<WidgetRenderingResult...> const& widgets, object_dict const& dict)
	{
		auto const theme_updated_callbacks = widgets.theme_updated_callbacks();
		auto const widget_pointers = widgets.widget_pointers();

		auto const n = std::size(widgets);
		for(auto k  = widgets.first_element_index(); k != n; ++k)
		{ theme_updated_callbacks[k](widget_pointers[k], dict); }
	}

	template<class WidgetRenderingResult>
	class widgets_to_render_list
	{
	public:
		using widget_array = multi_array<
			void*,
			prepare_for_presentation_callback<WidgetRenderingResult>,
			WidgetRenderingResult,
			widget_geometry
		>;

		using index_type = typename widget_array::index_type;

		static constexpr index_type npos{static_cast<size_t>(-1)};

		explicit widgets_to_render_list(widget_list<WidgetRenderingResult> const& from)
		{ collect_widgets(from, m_objects); }

		constexpr auto first_element_index() const
		{ return m_objects.first_element_index(); }

		auto size() const
		{ return std::size(m_objects); }

		auto widget_pointers() const
		{ return m_objects.template get<0>(); }

		auto output_rectangles() const
		{ return m_objects.template get<2>(); }

		auto output_rectangles()
		{ return m_objects.template get<2>(); }

		auto widget_geometries() const
		{ return m_objects.template get<3>(); }

		auto widget_geometries()
		{ return m_objects.template get<3>(); }

		auto render_callbacks() const
		{ return m_objects.template get<1>(); }

	private:
		static void collect_widgets(widget_list<WidgetRenderingResult> const& from, widget_array& to)
		{
			auto const widget_pointers = from.widget_pointers();
			auto const widget_visibilities = from.widget_visibilities();
			// TODO: auto const children_callbacks = from.children_callbacks();
			auto const render_callbacks = from.render_callbacks();
			auto const widget_geometries = from.widget_geometries();

			auto const n = std::size(from);
			for(auto k = from.first_element_index(); k != n; ++k)
			{
				if(widget_visibilities[k] == widget_visibility::visible) [[likely]]
				{
					to.push_back(widget_pointers[k], render_callbacks[k], WidgetRenderingResult{}, widget_geometries[k]);
					// TODO: collect_widgets(children_callbacks[k](widget_pointers[k]), to);
				}
			}
		}

		widget_array m_objects;
	};

	template<class WidgetRenderingResult>
	void prepare_for_presentation(widgets_to_render_list<WidgetRenderingResult>& widgets)
	{
		auto const render_callbacks = widgets.render_callbacks();
		auto const widget_pointers = widgets.widget_pointers();
		auto output_rectangles = widgets.output_rectangles();

		auto const n = std::size(widgets);
		for(auto k = widgets.first_element_index(); k != n; ++k)
		{ render_callbacks[k](widget_pointers[k], output_rectangles[k]); }
	}

	template<class Renderer, class WidgetRenderingResult>
	void show_widgets(Renderer&& renderer, widgets_to_render_list<WidgetRenderingResult> const& widgets)
	{
		auto const widget_geometries = widgets.widget_geometries();
		auto const output_rects = widgets.output_rectangles();

		auto const n = std::size(widgets);
		for(auto k  = widgets.first_element_index(); k != n; ++k)
		{
			renderer.render(
				widget_geometries[k].where,
				widget_geometries[k].origin,
				widget_geometries[k].size,
				output_rects[k]
			);
		}
	}
}

#endif
