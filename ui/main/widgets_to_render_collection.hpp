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

		auto element_indices() const
		{ return m_objects.element_indices(); }

		auto size() const
		{ return std::size(m_objects); }

		auto widget_pointers() const
		{ return m_objects.template get<0>(); }

		auto render_callbacks() const
		{ return m_objects.template get<1>(); }

		auto widget_layers() const
		{ return m_objects.template get<2>(); }

		auto widget_layers()
		{ return m_objects.template get<2>(); }

		auto output_rectangles()
		{ return m_objects.template get<2>(); }

		auto widget_geometries() const
		{ return m_objects.template get<3>(); }

		auto widget_geometries()
		{ return m_objects.template get<3>(); }


	private:
		static void collect_widgets(widget_collection_view const& from, widget_array& to)
		{
			auto const widget_pointers = from.widget_pointers();
			auto const widget_states = from.widget_states();
			auto const children_callbacks = from.get_children_const_callbacks();
			auto const render_callbacks = from.render_callbacks();
			auto const widget_geometries = from.widget_geometries();

			for(auto k : from.element_indices())
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

	template<class WidgetRenderingResult, class GraphicsResourceFactory>
	void prepare_for_presentation(widgets_to_render_collection<WidgetRenderingResult>& widgets, GraphicsResourceFactory& res_factory)
	{
		auto const render_callbacks = widgets.render_callbacks();
		auto const widget_pointers = widgets.widget_pointers();
		auto widget_layers = widgets.widget_layers();

		for(auto k : widgets.element_indices())
		{ render_callbacks[k](widget_pointers[k], widget_layers[k], res_factory); }
	}

	template<class Renderer, class WidgetRenderingResult>
	void show_widgets(Renderer&& renderer, widgets_to_render_collection<WidgetRenderingResult> const& widgets)
	{
		auto const widget_geometries = widgets.widget_geometries();
		auto const widget_layers = widgets.widget_layers();

		for(auto k : widgets.element_indices())
		{
			renderer.render(
				widget_geometries[k].where,
				widget_geometries[k].origin,
				widget_geometries[k].size,
				widget_layers[k]
			);
		}
	}
}

#endif
