#ifndef TERRAFORMER_UI_MAIN_EVENT_DISPATCHER_HPP
#define TERRAFORMER_UI_MAIN_EVENT_DISPATCHER_HPP

#include "./widget.hpp"
#include "./events.hpp"

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
	struct event_dispatcher
	{
	public:
		template<auto WindowId>
		void error_detected(error_message const& msg) noexcept
		{ value_of(m_error_handler).template error_detected<WindowId>(msg); }

		template<auto WindowId>
		void handle_mouse_button_event(mouse_button_event const& event)
		{ value_of(m_widget_collection).handle_event(event); }

		template<auto WindowId>
		void handle_cursor_motion_event(cursor_motion_event const& event)
		{ value_of(m_widget_collection).handle_event(event); }

		template<auto WindowId>
		void window_is_closing()
		{ value_of(m_window_controller).template window_is_closing<WindowId>(); }

		template<auto WindowId>
		void handle_cursor_enter_leave_event(cursor_enter_leave_event const& event)
		{ value_of(m_window_controller).template cursor_at_window_boundary<WindowId>(event); }


		template<auto WindowId>
		void framebuffer_size_changed(fb_size size)
		{
			if(!m_theme_is_up_to_date) [[unlikely]]
			{
				value_of(m_widget_collection).theme_updated(m_resources);
				m_theme_is_up_to_date = true;
			}

			value_of(m_content_renderer)
				.set_viewport(0, 0, size.width, size.height)
				.set_world_transform(location{-1.0f, 1.0f, 0.0f}, size);
			value_of(m_frame_renderer)
				.set_viewport(0, 0, size.width, size.height)
				.set_world_transform(location{-1.0f, 1.0f, 0.0f}, size);
			value_of(m_widget_collection).handle_event(size);
		}

		template<class Viewport, class ... Overlay>
		bool operator()(Viewport&& viewport, Overlay&&... overlay)
		{
			value_of(m_content_renderer).clear_buffers();
			render();
			(...,overlay());
			value_of(viewport).swap_buffers();
			return value_of(m_window_controller).main_loop_should_exit(viewport);
		}

		void render()
		{
			auto const box_size = value_of(m_widget_collection).update_geometry();

			value_of(m_widget_collection).prepare_for_presentation(m_output_rectangle);
			value_of(m_content_renderer).render(
				location{0.0f, 0.0f, 0.0f},
				location{-1.0f, 1.0f, 0.0f},
				scaling{
					box_size.width.min,
					box_size.height.min,
					1.0f
				},
				m_output_rectangle
			);
			value_of(m_widget_collection).show_widgets(value_of(m_content_renderer));
		//	value_of(m_widget_collection).decorate_widgets(value_of(m_frame_renderer), textures, look);
		}

		object_dict m_resources;
		WidgetContainer m_widget_collection;
		WindowController m_window_controller;
		ContentRenderer m_content_renderer;
		FrameRenderer m_frame_renderer;
		ErrorHandler m_error_handler;
		typename dereferenced_type<ContentRenderer>::input_rectangle m_output_rectangle{};
		bool m_theme_is_up_to_date = false;
	};
}


#endif
