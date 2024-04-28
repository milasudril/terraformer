#ifndef TERRAFORMER_UI_MAIN_WIDGET_LIST_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_LIST_HPP

#include "./widget.hpp"
#include "lib/array_classes/multi_array.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"

namespace terraformer::ui::main
{
	template<renderer Renderer>
	class widget_list
	{
	public:
		using output_rectangle = typename Renderer::input_rectangle;
		using texture_repo = typename Renderer::texture_repo;
		using render_callback = void (*)(void*, output_rectangle&, texture_repo const&,theming::widget_look const&);
		using cursor_enter_leave_callback = void (*)(void*, wsapi::cursor_enter_leave_event const&);
		using cursor_position_callback = bool (*)(void*, wsapi::cursor_motion_event const&);
		using mouse_button_callback = bool (*)(void*, wsapi::mouse_button_event const&);
		using size_callback = wsapi::fb_size (*)(void*, wsapi::fb_size);

		using widget_array = multi_array<
			void*,
			output_rectangle,
			widget_visibility,
			widget_geometry,
			render_callback,
 			cursor_enter_leave_callback,
			cursor_position_callback,
			mouse_button_callback,
			size_callback
		>;

		using index_type = typename widget_array::index_type;

		static constexpr index_type npos{static_cast<size_t>(-1)};

		template<widget<Renderer> Widget>
		widget_list& append(
			std::reference_wrapper<Widget> w,
			widget_geometry const& initial_geometry,
			widget_visibility initial_visibility = widget_visibility::visible
		)
		{
			m_objects.push_back(
				&w.get(),
				output_rectangle{},
				initial_visibility,
				initial_geometry,
				[](
					void* obj,
					output_rectangle& rect,
					texture_repo const& textures,
					theming::widget_look const& look
				) -> void {
					return static_cast<Widget*>(obj)->render(rect, textures, look);
				},
				[](void* obj, wsapi::cursor_enter_leave_event const& event) -> void{
					static_cast<Widget*>(obj)->handle_event(event);
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

		auto size_callbacks() const
		{ return m_objects.template get<8>(); }

	private:
		widget_array m_objects;
	};

	template<renderer Renderer>
	void render_widgets(
		widget_list<Renderer>& widgets,
		typename widget_list<Renderer>::texture_repo const& textures,
		theming::widget_look const& look
	)
	{
		auto const render_callbacks = widgets.render_callbacks();
		auto const widget_pointers = widgets.widget_pointers();
		auto const widget_visibilities = widgets.widget_visibilities();
		auto output_rectangles = widgets.output_rectangles();

		auto const n = std::size(widgets);
		for(auto k = widgets.first_element_index(); k != n; ++k)
		{
			if(widget_visibilities[k] == widget_visibility::visible) [[likely]]
			{ render_callbacks[k](widget_pointers[k], output_rectangles[k], textures, look); }
		}
	}

	template<renderer Renderer>
	void show_widgets(Renderer&& renderer, widget_list<Renderer> const& widgets)
	{
		auto const widget_geometries = widgets.widget_geometries();
		auto const widget_visibilities = widgets.widget_visibilities();
		auto const output_rects = widgets.output_rectangles();

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
