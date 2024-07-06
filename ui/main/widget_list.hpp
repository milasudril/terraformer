#ifndef TERRAFORMER_UI_MAIN_WIDGET_LIST_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_LIST_HPP

#include "./widget.hpp"
#include "lib/array_classes/multi_array.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"

namespace terraformer::ui::main
{
	template<class ... WidgetRenderingResult>
	class widget_list
	{
	public:
		using cursor_enter_leave_callback = void (*)(void*, cursor_enter_leave_event const&);
		using size_constraints_callback = widget_size_constraints (*)(void const*);
		using size_callback = void (*)(void*, fb_size);
		using theme_updated_callback = void (*)(void*, object_dict const&);

		using widget_array = multi_array<
			void*,
			WidgetRenderingResult...,
			widget_visibility,
			widget_geometry,
			void (*)(void* obj,
				WidgetRenderingResult& rect,
				widget_instance_info const& instance_info,
				object_dict const& render_resources
			)...,
			cursor_enter_leave_callback,
			cursor_motion_event_callback,
			mouse_button_event_callback,
			size_constraints_callback,
			size_callback,
			theme_updated_callback
		>;

		using index_type = typename widget_array::index_type;

		static constexpr index_type npos{static_cast<size_t>(-1)};

		template<class Widget>
		requires widget<Widget, WidgetRenderingResult...>
		widget_list& append(
			std::reference_wrapper<Widget> w,
			widget_geometry const& initial_geometry,
			widget_visibility initial_visibility = widget_visibility::visible
		)
		{
			m_objects.push_back(
				&w.get(),
				WidgetRenderingResult{}...,
				initial_visibility,
				initial_geometry,
				[](
					void* obj,
					WidgetRenderingResult& rect,
					widget_instance_info const& instance_info,
					object_dict const& render_resources
				) -> void {
					return static_cast<Widget*>(obj)->prepare_for_presentation(rect, instance_info, render_resources);
				}...,
				[](void* obj, cursor_enter_leave_event const& event) -> void{
					static_cast<Widget*>(obj)->handle_event(event);
				},
				[](void* obj, cursor_motion_event const& event, input_device_grab& grab) -> void{
					static_cast<Widget*>(obj)->handle_event(event, grab);
				},
				[](void* obj, mouse_button_event const& mbe, input_device_grab& grab) -> void {
					static_cast<Widget*>(obj)->handle_event(mbe, grab);
				},
				[](void const* obj) -> widget_size_constraints {
					return static_cast<Widget const*>(obj)->get_size_constraints();
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

		template<size_t Index>
		auto output_rectangles() const
		{ return m_objects.template get<1 + Index>(); }

		template<size_t Index>
		auto output_rectangles()
		{ return m_objects.template get<1 + Index>(); }

		auto widget_visibilities() const
		{ return m_objects.template get<1 + sizeof...(WidgetRenderingResult)>(); }

		auto widget_visibilities()
		{ return m_objects.template get<1 + sizeof...(WidgetRenderingResult)>(); }

		auto widget_geometries() const
		{ return m_objects.template get<2 + sizeof...(WidgetRenderingResult)>(); }

		auto widget_geometries()
		{ return m_objects.template get<2 + sizeof...(WidgetRenderingResult)>(); }

		template<size_t OutputIndex>
		auto render_callbacks() const
		{ return m_objects.template get<3 + sizeof...(WidgetRenderingResult) + OutputIndex>(); }

		auto cursor_enter_leave_callbacks() const
		{ return m_objects.template get<3 + 2*sizeof...(WidgetRenderingResult)>(); }

		auto cursor_motion_callbacks() const
		{ return m_objects.template get<4 + 2*sizeof...(WidgetRenderingResult)>(); }

		auto mouse_button_callbacks() const
		{ return m_objects.template get<5 + 2*sizeof...(WidgetRenderingResult)>(); }

		auto size_constraints_callbacks() const
		{ return m_objects.template get<6 + 2*sizeof...(WidgetRenderingResult)>(); }

		auto size_callbacks() const
		{ return m_objects.template get<7 + 2*sizeof...(WidgetRenderingResult)>(); }

		auto const theme_updated_callbacks() const
		{ return m_objects.template get<8 + 2*sizeof...(WidgetRenderingResult)>(); }

	private:
		widget_array m_objects;
	};

	template<size_t OutputIndex, class ...WidgetRenderingResult>
	void prepare_widgets_for_presentation(
		widget_list<WidgetRenderingResult...>& widgets,
 		widget_instance_info const& widget_instance,
		object_dict const& render_resources
	)
	{
		auto const render_callbacks = widgets.template render_callbacks<OutputIndex>();
		auto const widget_pointers = widgets.widget_pointers();
		auto const widget_visibilities = widgets.widget_visibilities();
		auto output_rectangles = widgets.template output_rectangles<OutputIndex>();

		auto const n = std::size(widgets);
		for(auto k = widgets.first_element_index(); k != n; ++k)
		{
			if(widget_visibilities[k] == widget_visibility::visible) [[likely]]
			{
				render_callbacks[k](
					widget_pointers[k],
					output_rectangles[k],
					widget_instance_info{
						.section_level = widget_instance.section_level,
						.paragraph_index = k.get()
					},
					render_resources
				);
			}
		}
	}

	template<size_t OutputIndex, class Renderer, class ...WidgetRenderingResult>
	void show_widgets(Renderer&& renderer, widget_list<WidgetRenderingResult...> const& widgets)
	{
		auto const widget_geometries = widgets.widget_geometries();
		auto const widget_visibilities = widgets.widget_visibilities();
		auto const output_rects = widgets.template output_rectangles<OutputIndex>();

		auto const n = std::size(widgets);
		for(auto k  = widgets.first_element_index(); k != n; ++k)
		{
			if(widget_visibilities[k] == widget_visibility::visible) [[likely]]
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

	template<class Renderer, class ...WidgetRenderingResult, class StyleGenerator>
	void decorate_widgets(Renderer&& renderer,
		widget_list<WidgetRenderingResult...> const& widgets,
		StyleGenerator&& style
	)
	{
		auto const widget_geometries = widgets.widget_geometries();
		auto const widget_visibilities = widgets.widget_visibilities();

		auto const n = std::size(widgets);
		for(auto k  = widgets.first_element_index(); k != n; ++k)
		{
			if(widget_visibilities[k] == widget_visibility::visible) [[likely]]
			{
				renderer.render(
					widget_geometries[k].where,
					widget_geometries[k].origin,
					widget_geometries[k].size,
					style(k)
				);
			}
		}
	}

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
}

#endif
