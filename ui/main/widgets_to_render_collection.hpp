#ifndef TERRAFORMER_UI_MAIN_WIDGETS_TO_RENDER_COLLECTION_HPP
#define TERRAFORMER_UI_MAIN_WIDGETS_TO_RENDER_COLLECTION_HPP

#include "lib/array_classes/multi_array.hpp"

namespace terraformer::ui::main
{
	template<class WidgetRenderingResult>
	class widgets_to_render_collection
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

		explicit widgets_to_render_collection(widget_collection_view const& from)
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
			auto const widget_states = from.widget_states();
			auto const children_callbacks = from.get_children_const_callbacks();
			auto const render_callbacks = from.render_callbacks();
			auto const widget_geometries = from.widget_geometries();

			auto const n = std::size(from);
			for(auto k = from.first_element_index(); k != n; ++k)
			{
				if(!widget_states[k].collapsed) [[likely]]
				{
					to.push_back(widget_pointers[k], render_callbacks[k], WidgetRenderingResult{}, widget_geometries[k]);
					collect_widgets(children_callbacks[k](widget_pointers[k]), to);
				}
			}
		}

		widget_array m_objects;
	};

	template<class WidgetRenderingResult>
	void prepare_for_presentation(widgets_to_render_collection<WidgetRenderingResult>& widgets)
	{
		auto const render_callbacks = widgets.render_callbacks();
		auto const widget_pointers = widgets.widget_pointers();
		auto output_rectangles = widgets.output_rectangles();

		auto const n = std::size(widgets);
		for(auto k = widgets.first_element_index(); k != n; ++k)
		{ render_callbacks[k](widget_pointers[k], widget_rendering_result{std::ref(output_rectangles[k])}); }
	}

	template<class Renderer, class WidgetRenderingResult>
	void show_widgets(Renderer&& renderer, widgets_to_render_collection<WidgetRenderingResult> const& widgets)
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