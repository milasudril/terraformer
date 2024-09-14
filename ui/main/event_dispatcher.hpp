#ifndef TERRAFORMER_UI_MAIN_EVENT_DISPATCHER_HPP
#define TERRAFORMER_UI_MAIN_EVENT_DISPATCHER_HPP

#include "./widget.hpp"
#include "./widgets_to_render_collection.hpp"
#include "./events.hpp"
#include "./widget_collection.hpp"

#include "lib/common/value_accessor.hpp"

constinit size_t event_count = 0;

namespace terraformer::ui::main
{
	template<
		class WindowController,
		class ContentRenderer,
		class FrameRenderer,
		class ErrorHandler
	>
	struct event_dispatcher
	{
	public:
		template<class Tag>
		void handle_event(Tag, error_message const& msg) noexcept
		{ value_of(m_error_handler).handle_event(Tag{}, msg); }

		template<class Tag>
		void handle_event(Tag, mouse_button_event const& event)
		{
			auto res = find_recursive(event.where, m_root_collection.get_attributes());
			if(!try_dispatch(event, res))
			{ printf("mbe in the void %zu\n", event_count); }

			++event_count;
		}

		template<class Tag>
		void handle_event(Tag, cursor_motion_event const& event)
		{
			auto res = find_recursive(event.where, m_root_collection.get_attributes());
			if(res != m_hot_widget)
			{
				if(!try_dispatch(cursor_leave_event{.where = event.where}, m_hot_widget))
				{ printf("cursor left the void %zu\n", event_count); }

				if(!try_dispatch(cursor_enter_event{.where = event.where}, res))
				{ printf("cursor entered the void %zu\n", event_count); }

				m_hot_widget = res;
			}

			if(!try_dispatch(event, res))
			{ printf("cme in the void %zu\n", event_count); }
			++event_count;
		}

		template<class Tag>
		void handle_event(Tag, window_close_event event)
		{ value_of(m_window_controller).handle_event(Tag{}, event); }

		template<class Tag>
		void handle_event(Tag, cursor_enter_event const& event)
		{ value_of(m_window_controller).handle_event(Tag{}, event); }

		template<class Tag>
		void handle_event(Tag, cursor_leave_event const& event)
		{ value_of(m_window_controller).handle_event(Tag{}, event); }


		template<class Tag>
		void handle_event(Tag, fb_size size)
		{
			if(!m_theme_is_up_to_date) [[unlikely]]
			{
				theme_updated(std::as_const(m_root_collection).get_attributes(), m_resources);
				m_theme_is_up_to_date = true;
			}

			value_of(m_content_renderer)
				.set_viewport(0, 0, size.width, size.height)
				.set_world_transform(location{-1.0f, 1.0f, 0.0f}, size);
			value_of(m_frame_renderer)
				.set_viewport(0, 0, size.width, size.height)
				.set_world_transform(location{-1.0f, 1.0f, 0.0f}, size);

			// TODO: Should update size here as well
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
			root_widget root{m_root_collection.get_attributes(), m_root_collection.first_element_index()};
			// TODO: Pick width/height based on window size
			auto const box_size = minimize_width(compute_size_constraints(root));
			confirm_sizes(
				root,
				fb_size{
					.width = static_cast<int>(box_size[0]),
					.height = static_cast<int>(box_size[1])
				}
			);

			m_root_collection.get_attributes().widget_geometries().front() = widget_geometry{
				.where = location{0.0f, 0.0f, 0.0f},
				.origin = location{-1.0f, 1.0f, 0.0f},
				.size = box_size
			};
			apply_offsets(root, displacement{0.0f, 0.0f, 0.0f});

			using WidgetRenderingResult = typename dereferenced_type<ContentRenderer>::input_rectangle;
			main::widgets_to_render_collection<WidgetRenderingResult>
				widgets_to_render{std::as_const(m_root_collection).get_attributes()};

			prepare_for_presentation(widgets_to_render);

			show_widgets(value_of(m_content_renderer), widgets_to_render);
			if(m_hot_widget != find_recursive_result{} && m_hot_widget.state().has_cursor_focus_indicator())
			{
				auto const& geometry = m_hot_widget.geometry();
				using Frame = typename FrameRenderer::input_rectangle;
				m_frame_renderer.render(
					geometry.where + 2.0f*displacement{-1.0f, 1.0f, 0.0f},
					geometry.origin,
					geometry.size + 4.0f*scaling{1.0f, 1.0f, 0.0f},
					Frame{
						.thickness = 2.0f,
						.texture = nullptr,
						.tints = std::array<rgba_pixel, 8>{}
					}
				);
			}
		}

		object_dict m_resources;
		WindowController m_window_controller;
		ContentRenderer m_content_renderer;
		FrameRenderer m_frame_renderer;
		ErrorHandler m_error_handler;

		bool m_theme_is_up_to_date = false;
		find_recursive_result m_hot_widget{};

		// TODO: Currently, a collection is used here, even though only one widget can be supported.
		// A widget collection is currently necessary to set m_hot_widget properly
		widget_collection m_root_collection{};

	};
}


#endif
