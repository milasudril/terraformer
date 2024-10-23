#ifndef TERRAFORMER_UI_MAIN_EVENT_DISPATCHER_HPP
#define TERRAFORMER_UI_MAIN_EVENT_DISPATCHER_HPP

#include "ui/main/widget.hpp"
#include "ui/main/widgets_to_render_collection.hpp"
#include "ui/main/events.hpp"
#include "ui/main/widget_collection.hpp"
#include "ui/main/flat_widget_collection.hpp"
#include "ui/main/window_ref.hpp"
#include "ui/main/ui_controller.hpp"

#include "lib/common/value_accessor.hpp"

namespace terraformer::ui::main
{
	constexpr span_search_direction get_form_navigation_step_size(keyboard_button_event const& kbe)
	{
		constexpr auto tab = 0x0f;
		if(
			kbe.scancode == tab &&
			(kbe.action == keyboard_button_action::press || kbe.action == keyboard_button_action::repeat)
		)
		{
			switch(kbe.modifiers & ~(main::modifier_keys::numlock | main::modifier_keys::capslock))
			{
				case main::modifier_keys::shift:
					return span_search_direction::backwards;
				case main::modifier_keys::none:
					return span_search_direction::forwards;
				default:
					return span_search_direction::stay;
			}
		}
		return span_search_direction::stay;
	}

	template<
		class WindowController,
		class ContentRenderer,
		class FrameRenderer,
		class ErrorHandler
	>
	class event_dispatcher
	{
	public:
		template<class Cfg, class Wc, class Cr, class Fr, class Eh, widget Widget>
		explicit event_dispatcher(
			Cfg&& cfg,
			Wc&& wc,
			Cr&& cr,
			Fr&& fr,
			Eh&& eh,
			std::reference_wrapper<Widget> root
		):
		m_config{std::forward<Cfg>(cfg)},
		m_window_controller{std::forward<Wc>(wc)},
		m_content_renderer{std::forward<Cr>(cr)},
		m_frame_renderer{std::forward<Fr>(fr)},
		m_error_handler{std::forward<Eh>(eh)}
		{
			m_root_collection.append(root, widget_geometry{});
			m_flat_collection = flatten(std::as_const(m_root_collection).get_attributes());
			main::theme_updated(m_root_collection, m_config);
		}

		template<class Tag>
		void handle_event(Tag, window_ref, error_message const& msg) noexcept
		{ value_of(m_error_handler).handle_event(Tag{}, msg); }

		template<class Tag>
		void handle_event(Tag, window_ref, mouse_button_event const& event)
		{
			auto const res = find_recursive(event.where, m_root_collection);

			if(!try_dispatch(event, res))
			{
				if(event.action == mouse_button_action::press)
				{ m_keyboard_widget = flat_widget_collection::npos; }
				printf("mbe in the void\n");
			}
			else
			if(event.action == mouse_button_action::press)
			{
				if(res.state().accepts_keyboard_input())
				{ set_keyboard_focus(find(res, m_flat_collection)); }
				else
				{ m_keyboard_widget = flat_widget_collection::npos; }
			}
		}

		template<class Tag>
		void handle_event(Tag, window_ref, cursor_motion_event const& event)
		{
			auto const res = find_recursive(event.where, m_root_collection);

			if(res != m_hot_widget)
			{
				if(!try_dispatch(cursor_leave_event{.where = event.where}, m_hot_widget))
				{ printf("cursor left the void\n"); }

				if(!try_dispatch(cursor_enter_event{.where = event.where}, res))
				{ printf("cursor entered the void\n"); }

				m_hot_widget = res;
			}

			if(!try_dispatch(event, res))
			{ printf("cme in the void\n"); }
		}

		template<class Tag>
		void handle_event(Tag, window_ref, keyboard_button_event const& event)
		{
			auto const nav_step = get_form_navigation_step_size(event);
			auto const next_widget = find_next_wrap_around(
				m_flat_collection.attributes().widget_states(),
				m_keyboard_widget,
				nav_step,
				[](auto item){
					return item.get().accepts_keyboard_input();
				}
			);

			if(next_widget == m_keyboard_widget)
			{
				if(!try_dispatch(event, m_flat_collection.attributes(), next_widget))
				{ printf("kbe in the void\n"); }
			}
			else
			{ set_keyboard_focus(next_widget); }
		}

		template<class Tag>
		void handle_event(Tag, window_ref, typing_event event)
		{
			if(!try_dispatch(event, m_flat_collection.attributes(), m_keyboard_widget))
			{ printf("%08x\n", event.codepoint); }
		}


		template<class Tag>
		void handle_event(Tag, window_ref, window_close_event event)
		{ value_of(m_window_controller).handle_event(Tag{}, event); }

		template<class Tag>
		void handle_event(Tag, window_ref, cursor_enter_event const& event)
		{ value_of(m_window_controller).handle_event(Tag{}, event); }

		template<class Tag>
		void handle_event(Tag, window_ref, cursor_leave_event const& event)
		{ value_of(m_window_controller).handle_event(Tag{}, event); }

		template<class Tag>
		void handle_event(Tag, window_ref, fb_size size)
		{
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

		void set_keyboard_focus(flat_widget_collection::index_type new_widget)
		{
			if(new_widget == m_keyboard_widget ||
				!(new_widget >= m_flat_collection.first_element_index() &&
				new_widget < std::size(m_flat_collection))
			)
			{ return; }

			try_dispatch(keyboard_focus_enter_event{}, m_flat_collection.attributes(), new_widget);

			m_keyboard_widget = new_widget;
		}

		void theme_updated(config&& new_config)
		{
			theme_updated(m_root_collection, new_config);
			m_config = std::move(new_config);
		}

		void render()
		{
			root_widget root{m_root_collection.get_attributes(), m_root_collection.first_element_index()};
			// TODO: Pick width/height based on window size
			auto const box_size = compute_size(root);
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
			if(m_hot_widget != find_recursive_result{}
				&& m_hot_widget.state().has_cursor_focus_indicator())
			{
				auto const& geometry = m_hot_widget.geometry();
				auto const color = m_config.mouse_kbd_tracking.colors.mouse_focus_color;
				auto const border_thickness = m_config.mouse_kbd_tracking.border_thickness;
				using Frame = typename FrameRenderer::input_rectangle;
				m_frame_renderer.render(
					geometry.where + border_thickness*displacement{-1.0f, 1.0f, 0.0f},
					geometry.origin,
					geometry.size + 2.0f*border_thickness*scaling{1.0f, 1.0f, 0.0f},
					Frame{
						.thickness = border_thickness,
						.texture = m_config.misc_textures.white.get_if<typename Frame::texture_type>(),
						.tints = std::array{
							0.0f*color,
							0.0f*color,
							0.0f*color,
							0.0f*color,
							color,
							color,
							color,
							color
						}
					}
				);
			}

			if(m_keyboard_widget != flat_widget_collection::npos)
			{
				auto const global_index = m_keyboard_widget;
				auto const address_array = m_flat_collection.attributes().addresses();
				auto const& keyboard_focus_item = address_array[global_index];

				auto const flat_attribs = keyboard_focus_item.collection();
				auto const local_index = keyboard_focus_item.index();
				auto const geoms = flat_attribs.widget_geometries();
				auto const& geometry = geoms[local_index];
				auto const color = m_config.mouse_kbd_tracking.colors.keyboard_focus_color;
				auto const border_thickness =  m_config.mouse_kbd_tracking.border_thickness;

				using Frame = typename FrameRenderer::input_rectangle;
				m_frame_renderer.render(
					geometry.where + border_thickness*displacement{-1.0f, 1.0f, 0.0f},
					geometry.origin,
					geometry.size + 2.0f*border_thickness*scaling{1.0f, 1.0f, 0.0f},
					Frame{
						.thickness = border_thickness,
						.texture = m_config.misc_textures.white.get_if<typename Frame::texture_type>(),
						.tints = std::array{
							0.0f*color,
							0.0f*color,
							0.0f*color,
							0.0f*color,
							color,
							color,
							color,
							color
						}
					}
				);
			}
		}

	private:
		config m_config;
		WindowController m_window_controller;
		ContentRenderer m_content_renderer;
		FrameRenderer m_frame_renderer;
		ErrorHandler m_error_handler;
		find_recursive_result m_hot_widget;
		flat_widget_collection::index_type m_keyboard_widget{flat_widget_collection::npos};

		// TODO: Currently, a collection is used here, even though only one widget can be supported.
		// A widget collection is currently necessary to set m_hot_widget properly
		widget_collection m_root_collection;
		flat_widget_collection m_flat_collection;
	};

		template<class Cfg, class Wc, class Cr, class Fr, class Eh, widget Widget>
		event_dispatcher(Cfg&&, Wc&&, Cr&&, Fr&&, Eh&&, std::reference_wrapper<Widget> root) ->
			event_dispatcher<Wc, Cr, Fr, Eh>;
}


#endif
