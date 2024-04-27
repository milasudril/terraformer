#ifndef TERRAFORMER_UI_MAIN_EVENT_DISPATCHER_HPP
#define TERRAFORMER_UI_MAIN_EVENT_DISPATCHER_HPP

#include "ui/wsapi/events.hpp"
#include "ui/drawing_api/single_quad_renderer.hpp"
#include "ui/theming/widget_look.hpp"
#include "lib/common/value_accessor.hpp"

namespace terraformer::ui::main
{
	template<class WidgetContainer, class WindowController, class Renderer, class ErrorHandler>
	class event_dispatcher
	{
	public:
		template<class T1, class T2, class T3, class T4>
		requires(
				 std::is_same_v<std::remove_cvref_t<T1>, WidgetContainer>
			&& std::is_same_v<std::remove_cvref_t<T2>, WindowController>
			&& std::is_same_v<std::remove_cvref_t<T3>, Renderer>
			&& std::is_same_v<std::remove_cvref_t<T4>, ErrorHandler>
		)
		explicit event_dispatcher(T1&& widget_container, T2&& window_controller, T3&& renderer, T4&& error_handler):
			m_widget_container{std::forward<T1>(widget_container)},
			m_window_controller{std::forward<T2>(window_controller)},
			m_renderer{std::forward<T3>(renderer)},
			m_error_handler{std::forward<T4>(error_handler)}
		{}

		template<auto WindowId>
		void error_detected(wsapi::error_message const& msg) noexcept
		{ value_of(m_error_handler).template error_detected<WindowId>(msg); }

		template<auto WindowId>
		void handle_mouse_button_event(wsapi::mouse_button_event const& event)
		{ value_of(m_widget_container).handle_event(event); }

		template<auto WindowId>
		void handle_cursor_motion_event(wsapi::cursor_motion_event const& event)
		{ value_of(m_widget_container).handle_event(event); }

		template<auto WindowId>
		void window_is_closing()
		{ value_of(m_window_controller).template window_is_closing<WindowId>(); }

		template<auto WindowId>
		void handle_cursor_enter_leave_event(ui::wsapi::cursor_enter_leave_event const& event)
		{ value_of(m_window_controller).template cursor_at_window_boundary<WindowId>(event); }

		template<auto WindowId>
		void framebuffer_size_changed(ui::wsapi::fb_size size)
		{
			m_fb_size = size;
			value_of(m_renderer)
				.set_viewport(0, 0, size.width, size.height)
				.set_world_transform(location{-1.0f, 1.0f, 0.0f}, size);
			value_of(m_widget_container).handle_event(size);
		}

		template<class Viewport>
		bool operator()(Viewport&& viewport, theming::widget_look const& look)
		{
			value_of(m_renderer).clear_buffers();
			render(look);
			value_of(viewport).swap_buffers();
			return value_of(m_window_controller).main_loop_should_exit(viewport);
		}

		void render(theming::widget_look const& look)
		{
			value_of(m_widget_container).render(m_output_rectangle, look);
			value_of(m_renderer).render(
				location{0.0f, 0.0f, 0.0f},
				location{-1.0f, 1.0f, 0.0f},
				scaling{static_cast<float>(m_fb_size.width), static_cast<float>(m_fb_size.height), 1.0f},
				m_output_rectangle
			);

			value_of(m_widget_container).show_widgets(value_of(m_renderer));
		}

	private:
		wsapi::fb_size m_fb_size{};
		WidgetContainer m_widget_container;
		WindowController m_window_controller;
		typename dereferenced_type<Renderer>::input_rectangle m_output_rectangle;
		Renderer m_renderer;
		ErrorHandler m_error_handler;
	};

	template<class T1, class T2, class T3, class T4>
	event_dispatcher(T1&&, T2&&, T3&&, T4&&) ->
	event_dispatcher<std::remove_cvref_t<T1>, std::remove_cvref_t<T2>, std::remove_cvref_t<T3>, std::remove_cvref_t<T4>>;
}


#endif