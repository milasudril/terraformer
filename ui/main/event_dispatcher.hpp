#ifndef TERRAFORMER_UI_MAIN_EVENT_DISPATCHER_HPP
#define TERRAFORMER_UI_MAIN_EVENT_DISPATCHER_HPP

#include "ui/wsapi/events.hpp"
#include "ui/drawing_api/single_quad_renderer.hpp"

namespace terraformer::ui::main
{
	template<class WidgetContainer, class WindowController, class ErrorHandler>
	class event_dispatcher
	{
	public:
		template<class T1, class T2, class T3>
		requires
				 std::is_same_v<std::decay_t<T1, WidgetContainer>>
			&& std::is_same_v<std::decay_t<T2, WindowController>>
			&& std::is_same_v<std::decay_t<T3, ErrorHandler>>
		explicit event_dispatcher(T1&& widget_container, T2&& window_controller, T3&& error_handler):
			m_widget_container{std::forward<T>(container)},
			m_window_controller{window_controller},
			m_error_handler{std::forward<T3>(error_handler)}
		{}

		void error_detected(wsapi::error_message const& msg) noexcept
		{ m_error_handler.handle(msg); }

		void handle_mouse_button_event(wsapi::mouse_button_event const& event)
		{ m_widget_container.handle_event(event); }

		// TODO: This will need to know about the current window
		void window_is_closing()
		{ m_window_controller.window_is_closing(); }

		// TODO: This will need to know about the current window
		void handle_cursor_enter_leave_event(wsapi::cursor_enter_leave_event const& event)
		{ m_window_controller.cursor_at_window_boundary(event);}

		void framebuffer_size_changed(terraformer::ui::wsapi::fb_size size)
		{
			m_fb_size = size;
			glViewport(0, 0, size.width, size.height);

			// TODO: Using default quad renderer works for now, but perhaps this should be customizable
			auto& renderer = drawing_api::single_quad_renderer::get_default_instance();
			renderer.set_world_transform(terraformer::location{-1.0f, 1.0f, 0.0f}, size);

			m_widget_container.handle_event(size);
		}

		bool operator()(wsapi::native_window<drawing_api::gl_surface_configuration>& viewport)
		{
			glClear(GL_COLOR_BUFFER_BIT);
			render();
			viewport.swap_buffers();
			return m_window_controller.main_loop_should_exit(viewport);
		}

		void render()
		{
			m_widget_container.render();
			// TODO: Using default quad renderer works for now, but perhaps this should be customizable
			auto& renderer = drawing_api::single_quad_renderer::get_default_instance();
			renderer.render(
				terraformer::location{0.0f, 0.0f, 0.0f},
				terraformer::location{-1.0f, 1.0f, 0.0f},
				terraformer::scaling{static_cast<float>(m_fb_size.width), static_cast<float>(m_fb_size.height), 1.0f},
				m_widget_container.background(),
				m_widget_container.foreground()
			);

			m_widget_container.show_widgets(renderer);
		}

	private:
		wsapi::fb_size m_fb_size;
		WidgetContainer m_widget_container;
		WindowController m_window_controller;
		ErrorHandler m_error_handler;
	};
}

#endif