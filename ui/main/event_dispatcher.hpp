#ifndef TERRAFORMER_UI_MAIN_EVENT_DISPATCHER_HPP
#define TERRAFORMER_UI_MAIN_EVENT_DISPATCHER_HPP

#include "ui/wsapi/events.hpp"
#include "ui/drawing_api/single_quad_renderer.hpp"
#include "ui/theming/widget_look.hpp"
#include "lib/common/value_accessor.hpp"

namespace terraformer::ui::main
{
	template<
		class WidgetContainer,
		class WindowController,
		class ContentRenderer,
		class FrameRenderer,
		class ErrorHandler
	>
	class event_dispatcher
	{
	public:
		template<class T1, class T2, class T3, class T4, class T5>
		requires(
				 std::is_same_v<std::remove_cvref_t<T1>, WidgetContainer>
			&& std::is_same_v<std::remove_cvref_t<T2>, WindowController>
			&& std::is_same_v<std::remove_cvref_t<T3>, ContentRenderer>
			&& std::is_same_v<std::remove_cvref_t<T4>, FrameRenderer>
			&& std::is_same_v<std::remove_cvref_t<T5>, ErrorHandler>
		)
		explicit event_dispatcher(
			T1&& widget_container,
			T2&& window_controller,
			T3&& content_renderer,
			T4&& frame_renderer,
			T5&& error_handler
		):
			m_widget_container{std::forward<T1>(widget_container)},
			m_window_controller{std::forward<T2>(window_controller)},
			m_content_renderer{std::forward<T3>(content_renderer)},
			m_frame_renderer{std::forward<T4>(frame_renderer)},
			m_error_handler{std::forward<T5>(error_handler)}
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
			value_of(m_widget_container).update_layout();
			[[maybe_unused]] auto size_constraints = value_of(m_widget_container).get_size_constraints();
			m_fb_size = size;
			value_of(m_content_renderer)
				.set_viewport(0, 0, size.width, size.height)
				.set_world_transform(location{-1.0f, 1.0f, 0.0f}, size);
			value_of(m_frame_renderer)
				.set_viewport(0, 0, size.width, size.height)
				.set_world_transform(location{-1.0f, 1.0f, 0.0f}, size);
			value_of(m_widget_container).handle_event(size);
		}

		template<class Viewport, class TextureRepo, class ... Overlay>
		bool operator()(Viewport&& viewport, TextureRepo const& textures, theming::widget_look const& look, Overlay&&... overlay)
		{
			value_of(m_content_renderer).clear_buffers();
			render(textures, look);
			(...,overlay());
			value_of(viewport).swap_buffers();
			return value_of(m_window_controller).main_loop_should_exit(viewport);
		}

		template<class TextureRepo>
		void render(TextureRepo const& textures, theming::widget_look const& look)
		{
			value_of(m_widget_container).render(m_output_rectangle, textures, look);
			value_of(m_content_renderer).render(
				location{0.0f, 0.0f, 0.0f},
				location{-1.0f, 1.0f, 0.0f},
				scaling{static_cast<float>(m_fb_size.width), static_cast<float>(m_fb_size.height), 1.0f},
				m_output_rectangle
			);
			value_of(m_widget_container).show_widgets(value_of(m_content_renderer));
			value_of(m_widget_container).decorate_widgets(value_of(m_frame_renderer), textures, look);
		}

	private:
		wsapi::fb_size m_fb_size{};
		WidgetContainer m_widget_container;
		WindowController m_window_controller;
		typename dereferenced_type<ContentRenderer>::input_rectangle m_output_rectangle;
		ContentRenderer m_content_renderer;
		FrameRenderer m_frame_renderer;
		ErrorHandler m_error_handler;
	};

	template<class T1, class T2, class T3, class T4, class T5>
	event_dispatcher(T1&&, T2&&, T3&&, T4&&, T5&&) ->
		event_dispatcher<
			std::remove_cvref_t<T1>,
			std::remove_cvref_t<T2>,
			std::remove_cvref_t<T3>,
			std::remove_cvref_t<T4>,
			std::remove_cvref_t<T5>
		>;
}


#endif
