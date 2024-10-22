#ifndef TERRAFORMER_UI_WSAPI_HPP
#define TERRAFORMER_UI_WSAPI_HPP

#include "./context.hpp"
#include "ui/main/events.hpp"
#include "lib/common/bitmask_enum.hpp"
#include "ui/main/window_ref.hpp"

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include <memory>
#include <stdexcept>
#include <cstdio>
#include <functional>
#include <cassert>

template<>
struct terraformer::ui::main::window_traits<GLFWwindow>
{
	static void set_title(GLFWwindow& window, char const* new_title)
	{ glfwSetWindowTitle(&window, new_title); }
};

namespace terraformer::ui::wsapi
{
	struct native_window_handle_deleter
	{
		void operator()(GLFWwindow* window) const
		{
			if(window != nullptr)
			{
				glfwDestroyWindow(window);
			}
		}
	};

	using window_handle = std::unique_ptr<GLFWwindow, native_window_handle_deleter>;

	struct no_api_config{};

	void prepare_surface(context&, no_api_config){}

	void activate_render_context(GLFWwindow*, no_api_config){}

	void swap_buffers(GLFWwindow*, no_api_config){}

	struct window_geometry_configuration
	{
		int width{800};
		int height{500};
	};

	enum class window_features:uint32_t{
		decorated = 0x1,
		initially_focused = 0x2,
		focus_on_show = 0x4,
		user_resizable = 0x8,
		initially_maximized = 0x10
	};

	consteval void enable_bitmask_operators(window_features){}

	struct window_configuration
	{
		window_geometry_configuration geometry;
		window_features features{
			 window_features::user_resizable
			|window_features::decorated
			|window_features::focus_on_show
		};
	};

	inline void enable(context&, window_configuration const& cfg)
	{
		glfwWindowHint(
			GLFW_DECORATED,
			is_set(cfg.features, window_features::decorated) ? GLFW_TRUE:GLFW_FALSE
		);
		glfwWindowHint(
			GLFW_FOCUSED,
			is_set(cfg.features, window_features::initially_focused) ? GLFW_TRUE:GLFW_FALSE
		);
		glfwWindowHint(
			GLFW_FOCUS_ON_SHOW,
			is_set(cfg.features, window_features::focus_on_show) ? GLFW_TRUE:GLFW_FALSE
		);
		glfwWindowHint(
			GLFW_RESIZABLE,
			is_set(cfg.features, window_features::user_resizable) ? GLFW_TRUE:GLFW_FALSE
		);
		glfwWindowHint(
			GLFW_MAXIMIZED,
			is_set(cfg.features, window_features::initially_maximized) ? GLFW_TRUE:GLFW_FALSE
		);
	}

	constexpr main::modifier_keys to_keymask(int mask)
	{
		main::modifier_keys ret{};
		if(mask & GLFW_MOD_SHIFT)
		{ ret |= main::modifier_keys::shift; }

		if(mask & GLFW_MOD_CONTROL)
		{ ret |= main::modifier_keys::control; }

		if(mask & GLFW_MOD_ALT)
		{ ret |= main::modifier_keys::alt; }

		if(mask & GLFW_MOD_SUPER)
		{ ret |= main::modifier_keys::super; }

		if(mask & GLFW_MOD_CAPS_LOCK)
		{ ret |= main::modifier_keys::capslock; }

		if(mask & GLFW_MOD_NUM_LOCK)
		{ ret |= main::modifier_keys::numlock; }

		return ret;
	}

	constexpr main::keyboard_button_action to_keyboard_button_action(int action)
	{
		switch(action)
		{
			case GLFW_PRESS:
				return main::keyboard_button_action::press;
			case GLFW_RELEASE:
				return main::keyboard_button_action::release;
			case GLFW_REPEAT:
				return main::keyboard_button_action::repeat;
			default:
				return main::keyboard_button_action::release;
		}
	}

	template<class T, class Tag, class EventType>
	concept can_handle_event = std::is_empty_v<Tag> && requires(T& obj, EventType&& event, main::window_ref window)
	{
		{obj.handle_event(Tag{}, window, std::forward<EventType>(event))} -> std::same_as<void>;
	};

	template<class RenderContextConfiguration = no_api_config>
	class native_window
	{
	public:
		explicit native_window(
			context& ctxt,
			char const* title,
			RenderContextConfiguration&& ctxt_cfg = RenderContextConfiguration{},
			window_configuration const& wincfg = window_configuration{}):
			m_ctxt_cfg{std::move(ctxt_cfg)}
		{
			enable(ctxt, wincfg);
			prepare_surface(ctxt, m_ctxt_cfg);
			m_window.reset(
				glfwCreateWindow(
					wincfg.geometry.width,
					wincfg.geometry.height,
					title,
					nullptr,
					nullptr
				)
			);

			if(m_window == nullptr)
			{ throw std::runtime_error{"Failed to create a window"}; }

			glfwSetInputMode(m_window.get(), GLFW_LOCK_KEY_MODS, GLFW_TRUE);
			activate_render_context();
		}

		void activate_render_context()
		{
			using wsapi::activate_render_context;
			activate_render_context(m_window.get(), m_ctxt_cfg);
		}

		void swap_buffers()
		{
			using wsapi::swap_buffers;
			swap_buffers(m_window.get(), m_ctxt_cfg);
		}

		template<class WindowTag, class EventHandler>
		void set_event_handler(std::reference_wrapper<EventHandler> eh)
		{
			static_assert(can_handle_event<EventHandler, WindowTag, main::error_message>);
			glfwSetWindowUserPointer(m_window.get(), &eh.get());
			if constexpr (can_handle_event<EventHandler, WindowTag, main::window_close_event>)
			{
				glfwSetWindowCloseCallback(
					m_window.get(),
					[](GLFWwindow* window){
						auto event_handler = static_cast<EventHandler*>(glfwGetWindowUserPointer(window));
						dispatch_event<WindowTag>(*event_handler, *window, main::window_close_event{});
 					}
				);
			}

			if constexpr (can_handle_event<EventHandler, WindowTag, main::fb_size>)
			{
				glfwSetFramebufferSizeCallback(
					m_window.get(),
					[](GLFWwindow* window, int w, int h){
						auto event_handler = static_cast<EventHandler*>(glfwGetWindowUserPointer(window));
						dispatch_event<WindowTag>(*event_handler, *window, main::fb_size{w, h});
					}
				);
				dispatch_event<WindowTag>(eh.get(), *m_window.get(), get_fb_size());
			}

			if constexpr (can_handle_event<EventHandler, WindowTag, main::mouse_button_event>)
			{
				glfwSetMouseButtonCallback(
					m_window.get(),
					[](GLFWwindow* window, int button, int action, int modifiers){
						auto event_handler = static_cast<EventHandler*>(glfwGetWindowUserPointer(window));
						assert(action == GLFW_PRESS || action == GLFW_RELEASE);
						dispatch_event<WindowTag>(
							*event_handler,
							*window,
							main::mouse_button_event{
								.where = get_cursor_position(window),
								.button = button,
								.action = action == GLFW_PRESS? main::mouse_button_action::press : main::mouse_button_action::release,
								.modifiers = to_keymask(modifiers)
							}
						);
					}
				);
			}

			if constexpr (can_handle_event<EventHandler, WindowTag, main::cursor_motion_event>)
			{
				glfwSetCursorPosCallback(
					m_window.get(),
					[](GLFWwindow* window, double x, double y)
					{
						auto event_handler = static_cast<EventHandler*>(glfwGetWindowUserPointer(window));
						dispatch_event<WindowTag>(
							*event_handler,
							*window,
							main::cursor_motion_event{
								.where{
									.x = x,
									.y = -y
								}
							}
						);
					}
				);
			}

			if constexpr (
				   can_handle_event<EventHandler, WindowTag, main::cursor_enter_event>
				|| can_handle_event<EventHandler, WindowTag, main::cursor_leave_event>
			)
			{
				static_assert(can_handle_event<EventHandler, WindowTag, main::cursor_enter_event>);
				static_assert(can_handle_event<EventHandler, WindowTag, main::cursor_leave_event>);
				glfwSetCursorEnterCallback(
					m_window.get(),
					[](GLFWwindow* window, int direction){
						auto event_handler = static_cast<EventHandler*>(glfwGetWindowUserPointer(window));
						assert(direction == GLFW_TRUE || direction == GLFW_FALSE);
						if(direction == GLFW_TRUE)
						{
							dispatch_event<WindowTag>(
								*event_handler,
								*window,
								main::cursor_enter_event{
									.where = get_cursor_position(window)
								}
							);
						}
						else
						{
							dispatch_event<WindowTag>(
								*event_handler,
								*window,
								main::cursor_leave_event{
									.where = get_cursor_position(window)
								}
							);
						}
					}
				);
			}

			if constexpr (can_handle_event<EventHandler, WindowTag, main::keyboard_button_event>)
			{
				glfwSetKeyCallback(
					m_window.get(),
					[](GLFWwindow* window, int, int scancode, int action, int mods){
						auto event_handler = static_cast<EventHandler*>(glfwGetWindowUserPointer(window));
						dispatch_event<WindowTag>(
							*event_handler,
							*window,
							main::keyboard_button_event{
								.scancode = scancode - 8,
								.action = to_keyboard_button_action(action),
								.modifiers = to_keymask(mods)
							}
						);
					}
				);
			}

			if constexpr (can_handle_event<EventHandler, WindowTag, main::typing_event>)
			{
				glfwSetCharCallback(
					m_window.get(),
					[](GLFWwindow* window, uint32_t codepoint) {
						auto event_handler = static_cast<EventHandler*>(glfwGetWindowUserPointer(window));
						dispatch_event<WindowTag>(
							*event_handler,
							*window,
							main::typing_event{
								.codepoint = codepoint
							}
						);
					}
				);
			}
		}

		void set_window_title(char const* title)
		{	glfwSetWindowTitle(m_window.get(), title); }

		auto get_fb_size() const
		{
			main::fb_size ret{};
			glfwGetFramebufferSize(m_window.get(), &ret.width, &ret.height);
			return ret;
		}

		main::cursor_position get_cursor_position() const
		{ return get_cursor_position(m_window.get()); }

		auto handle() const { return m_window.get(); }

		void set_cursor(cursor const& new_cursor)
		{ glfwSetCursor(m_window.get(), new_cursor.handle()); }

	private:
		window_handle m_window;
		RenderContextConfiguration m_ctxt_cfg;

		static main::cursor_position get_cursor_position(GLFWwindow* window)
		{
			main::cursor_position ret{};
			glfwGetCursorPos(window, &ret.x, &ret.y);
			ret.y = -ret.y;
			return ret;
		};

		template<class WindowTag, class EventHandler, class... Args>
		static void dispatch_event(EventHandler& eh, GLFWwindow& window, Args&&... args)
		{
			try
			{ eh.handle_event(WindowTag{}, main::window_ref{window}, std::forward<Args>(args)...); }
			catch(std::exception const& e)
			{ eh.handle_event(WindowTag{}, main::window_ref{window}, main::error_message{e.what()}); }
			catch(char const* msg)
			{ eh.handle_event(WindowTag{}, main::window_ref{window}, main::error_message{msg}); }
			catch(...)
			{
				fprintf(stderr, "Caught unknown exception\n");
				fflush(stderr);
				abort();
			}
		}
	};
}

#endif
