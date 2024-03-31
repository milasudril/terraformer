#ifndef TERRAFORMER_UI_MAIN_WIDGET_LIST_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_LIST_HPP

#include "./widget.hpp"
#include "lib/array_classes/multi_array.hpp"

namespace terraformer::ui::main
{
	template<class DrawingSurface>
	class widget_list
	{
	public:
		using render_callback = void (*)(void const*, DrawingSurface& surface);
		using cursor_position_callback = bool (*)(void*, wsapi::cursor_position);
		using mouse_button_callback = bool (*)(void*, wsapi::mouse_button_event const& mbe);
		using size_callback = wsapi::fb_size (*)(void*, wsapi::fb_size);

		using widget_array = multi_array<
			void*,
			widget_visibility,
			widget_geometry,
			DrawingSurface,
			render_callback,
			cursor_position_callback,
			mouse_button_callback,
			size_callback
		>;
		
		using index_type = typename widget_array::index_type;

		static constexpr index_type npos{static_cast<size_t>(-1)};
		
		template<widget<DrawingSurface> Widget>
		widget_list& append(
			std::reference_wrapper<Widget> w,
			widget_geometry const& initial_geometry,
			widget_visibility initial_visibility = widget_visibility::visible
		)
		{
			m_objects.push_back(
				&w.get(),
				initial_visibility,
				initial_geometry,
				DrawingSurface{initial_geometry.width, initial_geometry.height},
				[](void const* obj, DrawingSurface& surface) -> void {
					return static_cast<Widget const*>(obj)->render_to(surface);
				},
				[](void* obj, wsapi::cursor_position pos) -> bool {
					return static_cast<Widget*>(obj)->handle_event(pos);
				},
				[](void* obj, wsapi::mouse_button_event const& mbe) ->bool {
					return static_cast<Widget*>(obj)->handle_event(mbe);
				},
				[](void* obj, wsapi::fb_size size)-> wsapi::fb_size {
					return static_cast<Widget*>(obj)->handle_event(size);
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
		
		auto widget_visibilities() const
		{ return m_objects.template get<1>(); }
		
		auto widget_visibilities()
		{ return m_objects.template get<1>(); }

		auto widget_geometries() const
		{ return m_objects.template get<2>(); }

		auto widget_geometries()		
		{ return m_objects.template get<2>(); }
		
		auto widget_surfaces()
		{ return m_objects.template get<3>(); }

		auto widget_surfaces() const
		{ return m_objects.template get<3>(); }

		auto render_callbacks() const
		{ return m_objects.template get<4>(); }

		auto cursor_position_callbacks() const
		{ return m_objects.template get<5>(); }

		auto mouse_button_callbacks() const
		{ return m_objects.template get<6>(); }

		auto size_callbacks() const
		{ return m_objects.template get<7>(); }

	private:		
		widget_array m_objects;
	};
	
	template<class DrawingSurface>
	void update_widget_surfaces(widget_list<DrawingSurface>& widgets)
	{
		auto const render_callbacks = widgets.render_callbacks();
		auto const widget_pointers = widgets.widget_pointers();
		auto const widget_surfaces = widgets.widget_surfaces();
		auto const widget_visibilities = widgets.widget_visibilities();
		auto const n = std::size(widgets);
		for(auto k = widgets.first_element_index(); k != n; ++k)
		{
			if(widget_visibilities[k] == widget_visibility::visible) [[likely]]
			{ render_callbacks[k](widget_pointers[k], widget_surfaces[k]); }
		}
	}
	
	template<class Renderer, class DrawingSurface>
	void show_widgets(Renderer&& renderer, widget_list<DrawingSurface> const& widgets)
	{
		auto const widget_surfaces = widgets.widget_surfaces();
		auto const widget_geometries = widgets.widget_geometries();
		auto const widget_visibilities = widgets.widget_visibilities();
		auto const n = std::size(widgets);
		for(auto k  = widgets.first_element_index(); k != n; ++k)
		{
			if(widget_visibilities[k] == widget_visibility::visible) [[likely]]
			{ renderer.render_surface(widget_surfaces[k], widget_geometries[k]); }
		}
	}
	
	inline auto find(wsapi::cursor_position pos, span<widget_geometry const> geoms)
	{
		return std::ranges::find_if(
			geoms,
			[pos](auto const& obj) {
				return inside(pos, obj);
			}
		);
	}
	
	template<class DrawingSurface>
	auto find(wsapi::cursor_position pos, widget_list<DrawingSurface> const& widgets)
	{
		auto const i = find(pos, widgets.widget_geometries());
		if(i == std::end(widgets.widget_geometries()))
		{ return widget_list<DrawingSurface>::npos; }
		
		return typename widget_list<DrawingSurface>::index_type{
			static_cast<size_t>(i - std::begin(widgets.widget_geometries()))
		};
	}
}

#endif
