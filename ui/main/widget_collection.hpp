#ifndef TERRAFORMER_UI_MAIN_WIDGET_COLLECTION_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_COLLECTION_HPP

#include "./widget.hpp"
#include "lib/array_classes/multi_array.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"

namespace terraformer::ui::main
{
	class widget_collection
	{
	public:
		using widget_array = compatible_multi_array_t<widget_collection_ref::widget_span>;

		using index_type = widget_array::index_type;

		static constexpr index_type npos{static_cast<size_t>(-1)};

		template<class Widget>
		requires widget<Widget>
		widget_collection& append(
			std::reference_wrapper<Widget> w,
			widget_geometry const& initial_geometry,
			widget_visibility initial_visibility = widget_visibility::visible
		)
		{
			m_objects.push_back(
				&w.get(),
				initial_visibility,
				widget_size_constraints{},
				initial_geometry,
				[](void* obj, widget_rendering_result result) {
					return static_cast<Widget*>(obj)->prepare_for_presentation(result);
				},
				[](void* obj, cursor_enter_event const& event) {
					static_cast<Widget*>(obj)->handle_event(event);
				},
				[](void* obj, cursor_leave_event const& event) {
					static_cast<Widget*>(obj)->handle_event(event);
				},
				[](void* obj, cursor_motion_event const& event) {
					static_cast<Widget*>(obj)->handle_event(event);
				},
				[](void* obj, mouse_button_event const& mbe) {
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
				},
				[](void* obj){
					return static_cast<Widget*>(obj)->get_children();
				},
				[](void const* obj){
					return static_cast<Widget const*>(obj)->get_children();
				},
				[](void const* obj){
					return static_cast<Widget const*>(obj)->get_layout();
				}
			);

			return *this;
		}

		auto get_attributes()
		{ return widget_collection_ref{m_objects.attributes()}; }

		auto get_attributes() const
		{ return widget_collection_view{m_objects.attributes()}; }

	private:
		widget_array m_objects;
	};

	inline auto find(cursor_position pos, widget_collection const& widgets)
	{ return find(pos, widgets.get_attributes());}

	void theme_updated(widget_collection const& widgets, object_dict const& dict)
	{ theme_updated(widgets.get_attributes(), dict); }

	template<class WidgetRenderingResult>
	class widgets_to_render_list
	{
	public:
		using widget_array = multi_array<
			void*,
			prepare_for_presentation_callback,
			WidgetRenderingResult,
			widget_geometry
		>;

		using index_type = typename widget_array::index_type;

		static constexpr index_type npos{static_cast<size_t>(-1)};

		explicit widgets_to_render_list(widget_collection_view const& from)
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
		static void collect_widgets(widget_collection_view const& from, widget_array& to)
		{
			auto const widget_pointers = from.widget_pointers();
			auto const widget_visibilities = from.widget_visibilities();
			auto const children_callbacks = from.get_children_const_callbacks();
			auto const render_callbacks = from.render_callbacks();
			auto const widget_geometries = from.widget_geometries();

			auto const n = std::size(from);
			for(auto k = from.first_element_index(); k != n; ++k)
			{
				if(widget_visibilities[k] == widget_visibility::visible) [[likely]]
				{
					to.push_back(widget_pointers[k], render_callbacks[k], WidgetRenderingResult{}, widget_geometries[k]);
					collect_widgets(children_callbacks[k](widget_pointers[k]), to);
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
		{ render_callbacks[k](widget_pointers[k], widget_rendering_result{std::ref(output_rectangles[k])}); }
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
