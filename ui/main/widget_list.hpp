#ifndef TERRAFORMER_UI_MAIN_WIDGET_LIST_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_LIST_HPP

#include "ui/wsapi/native_window.hpp"
#include "lib/array_classes/multi_array.hpp"

namespace terraformer::ui::main
{
	template<class T, class RenderSurface>
	concept widget = requires(
		T& obj,
		wsapi::fb_size size,
		wsapi::cursor_position pos,
		wsapi::mouse_button_event const& mbe,
		RenderSurface& surface
	)
	{
		{ std::as_const(obj).render_to(surface) } -> std::same_as<void>;
		{ obj.handle_event(std::as_const(pos)) } -> std::same_as<bool>;
		{ obj.handle_event(mbe) } -> std::same_as<bool>;
		{ obj.handle_event(std::as_const(size)) } -> std::same_as<wsapi::fb_size>;
	};

	template<class RenderSurface>
	struct widget_with_default_actions
	{
		void render_to(RenderSurface&) const {}
		bool handle_event(wsapi::cursor_position) const { return false; }
		bool handle_event(wsapi::mouse_button_event const&) const { return false; }
		wsapi::fb_size handle_event(wsapi::fb_size size) const { return size; }
	};

	static_assert(widget<widget_with_default_actions<int>, int>);

	template<class RenderSurface>
	class widget_list
	{
	public:
		template<widget<RenderSurface> Widget>
		widget_list& append(std::reference_wrapper<Widget> w)
		{
			m_objects.push_back(
				std::in_place_t{},
				&w,
				RenderSurface{},
				[](void const* obj, RenderSurface& surface){
					return static_cast<Widget const*>(obj)->render_to(surface);
				},
				[](void* obj, wsapi::cursor_position pos){
					return static_cast<Widget*>(obj)->handle_event(pos);
				},
				[](void* obj, wsapi::mouse_button_event const& mbe){
					return static_cast<Widget*>(obj)->handle_event(mbe);
				},
				[](void* obj, wsapi::fb_size size){
					return static_cast<Widget*>(obj)->handle_event(size);
				}
			);

			return *this;
		}

		auto size() const
		{ return std::size(m_objects); }

		auto widgets() const
		{ return m_objects.template get<0>(); }

		auto surfaces() const
		{ return m_objects.template get<1>(); }

		auto render_callbacks() const
		{ return m_objects.template get<2>(); }

		auto cursor_position_callbacks() const
		{ return m_objects.template get<3>(); }

		auto mouse_button_callbacks() const
		{ return m_objects.template get<4>(); }

		auto size_callbacks() const
		{ return m_objects.template get<5>(); }

	private:
		using render_callback = void (*)(void const*, RenderSurface& surface);
		using cursor_position_callback = bool (*)(void*, wsapi::cursor_position);
		using mouse_button_callback = bool (*)(void*, wsapi::mouse_button_event const& mbe);
		using size_callback = wsapi::fb_size (*)(void*, wsapi::fb_size);

		multi_array<
			void*,
			RenderSurface,
			render_callback,
			cursor_position_callback,
			mouse_button_callback,
			size_callback
		> m_objects;
	};
}

#endif