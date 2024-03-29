#ifndef TERRAFORMER_UI_MAIN_WIDGET_LIST_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_LIST_HPP

#include "./widget.hpp"
#include "lib/array_classes/multi_array.hpp"

namespace terraformer::ui::main
{
	template<class RenderSurface>
	class widget_list
	{
	public:
		template<widget<RenderSurface> Widget>
		widget_list& append(std::reference_wrapper<Widget> w, widget_visibility visibility = Widget::default_visibility)
		{
			m_objects.push_back(
				&w,
				visibility,
				widget_geometry{},
				RenderSurface{},
				[](void const* obj, RenderSurface& surface) -> void{
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

		auto size() const
		{ return std::size(m_objects); }

		auto widgets() const
		{ return m_objects.template get<0>(); }
		
		auto widget_visibilities() const
		{ return m_objects.template get<1>(); }
		
		auto widget_visibilities()
		{ return m_objects.template get<1>(); }

		auto widget_geometries() const
		{ return m_objects.template get<2>(); }

		auto widget_geometries()
		{ return m_objects.template get<2>(); }

		auto surfaces() const
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
		using render_callback = void (*)(void const*, RenderSurface& surface);
		using cursor_position_callback = bool (*)(void*, wsapi::cursor_position);
		using mouse_button_callback = bool (*)(void*, wsapi::mouse_button_event const& mbe);
		using size_callback = wsapi::fb_size (*)(void*, wsapi::fb_size);

		multi_array<
			void*,
			widget_visibility,
			widget_geometry,
			RenderSurface,
			render_callback,
			cursor_position_callback,
			mouse_button_callback,
			size_callback
		> m_objects;
	};
}

#endif
