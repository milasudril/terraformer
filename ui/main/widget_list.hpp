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
		using render_callback = void (*)(void*);
		using drawing_surface_callback = DrawingSurface (*)(void const*);
		using cursor_position_callback = bool (*)(void*, wsapi::cursor_motion_event const&);
		using mouse_button_callback = bool (*)(void*, wsapi::mouse_button_event const&);
		using size_callback = wsapi::fb_size (*)(void*, wsapi::fb_size);

		using widget_array = multi_array<
			void*,
			widget_visibility,
			widget_geometry,
			render_callback,
			drawing_surface_callback,
			drawing_surface_callback,
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
				[](void* obj) -> void {
					return static_cast<Widget*>(obj)->render();
				},
				[](void const* obj) -> DrawingSurface {
					return static_cast<Widget const*>(obj)->background();
				},
				[](void const* obj) -> DrawingSurface {
					return static_cast<Widget const*>(obj)->foreground();
				},
				[](void* obj, wsapi::cursor_motion_event const& event) -> bool {
					return static_cast<Widget*>(obj)->handle_event(event);
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

		auto render_callbacks() const
		{ return m_objects.template get<3>(); }

		auto background_callbacks() const
		{ return m_objects.template get<4>(); }

		auto foreground_callbacks() const
		{ return m_objects.template get<5>(); }

		auto cursor_motion_callbacks() const
		{ return m_objects.template get<6>(); }

		auto mouse_button_callbacks() const
		{ return m_objects.template get<7>(); }

		auto size_callbacks() const
		{ return m_objects.template get<8>(); }

	private:
		widget_array m_objects;
	};

	template<class DrawingSurface>
	void render_widgets(widget_list<DrawingSurface>& widgets)
	{
		auto const render_callbacks = widgets.render_callbacks();
		auto const widget_pointers = widgets.widget_pointers();
		auto const widget_visibilities = widgets.widget_visibilities();
		auto const n = std::size(widgets);
		for(auto k = widgets.first_element_index(); k != n; ++k)
		{
			if(widget_visibilities[k] == widget_visibility::visible) [[likely]]
			{ render_callbacks[k](widget_pointers[k]); }
		}
	}

	template<class Renderer, class DrawingSurface>
	void show_widgets(Renderer&& renderer, widget_list<DrawingSurface> const& widgets)
	{
		auto const background_callbacks = widgets.background_callbacks();
		auto const foreground_callbacks = widgets.foreground_callbacks();
		auto const widget_pointers = widgets.widget_pointers();
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
					background_callbacks[k](widget_pointers[k]),
					foreground_callbacks[k](widget_pointers[k])
				);
			}
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
