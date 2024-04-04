#ifndef TERRAFORMER_UI_WSAPI_HPP
#define TERRAFORMER_UI_WSAPI_HPP

#include "./context.hpp"
#include "./events.hpp"
#include "lib/common/bitmask_enum.hpp"

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include <memory>
#include <stdexcept>
#include <cstdio>
#include <functional>
#include <cassert>

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
		initially_maximized = 0x10,
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

		template<class EventHandler>
		void set_event_handler(std::reference_wrapper<EventHandler> eh)
		{
			static_assert(requires(error_message const& msg){{eh.get().error_detected(msg)}->std::same_as<void>;});

			glfwSetWindowUserPointer(m_window.get(), &eh.get());
			if constexpr (requires{{eh.get().window_is_closing()}->std::same_as<void>;})
			{
				glfwSetWindowCloseCallback(m_window.get(), [](GLFWwindow* window) -> void {
					auto event_handler = static_cast<EventHandler*>(glfwGetWindowUserPointer(window));
					call_and_catch(&EventHandler::window_is_closing, *event_handler);
				});
			}

			if constexpr (requires(fb_size size){{eh.get().framebuffer_size_changed(size)}->std::same_as<void>;})
			{
				glfwSetFramebufferSizeCallback(
					m_window.get(),
					[](GLFWwindow* window, int w, int h){
						auto event_handler = static_cast<EventHandler*>(glfwGetWindowUserPointer(window));
						call_and_catch(&EventHandler::framebuffer_size_changed, *event_handler, fb_size{w, h});
					}
				);
				eh.get().framebuffer_size_changed(get_fb_size());
			}

			if constexpr (requires(mouse_button_event const& event){
				{eh.get().handle_mouse_button_event(event)}->std::same_as<void>;
			})
			{
				glfwSetMouseButtonCallback(m_window.get(),
					[](GLFWwindow* window, int button, int action, int modifiers){
						auto event_handler = static_cast<EventHandler*>(glfwGetWindowUserPointer(window));
						assert(action == GLFW_PRESS || action == GLFW_RELEASE);
						call_and_catch(
							&EventHandler::handle_mouse_button_event,
							*event_handler,
							mouse_button_event{
								.where = get_cursor_position(window),
								.button = button,
								.action = action == GLFW_PRESS? button_action::press : button_action::release,
								.modifiers = modifier_keys{modifiers}
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
			fb_size ret{};
			glfwGetFramebufferSize(m_window.get(), &ret.width, &ret.height);
			return ret;
		}

		cursor_position get_cursor_position() const
		{
			cursor_position ret{};
			glfwGetCursorPos(m_window.get(), &ret.x, &ret.y);
			return ret;
		}

		auto handle() const { return m_window.get(); }


	private:
		window_handle m_window;
		RenderContextConfiguration m_ctxt_cfg;

		static cursor_position get_cursor_position(GLFWwindow* window)
		{
			cursor_position ret{};
			glfwGetCursorPos(window, &ret.x, &ret.y);
			return ret;
		};

		template<class EventHandler, class Function, class ... Args>
		static void call_and_catch(Function&& f, EventHandler&& eh, Args&&... args)
		{
			try
			{ std::invoke(std::forward<Function>(f), std::forward<EventHandler>(eh), std::forward<Args>(args)...); }
			catch(std::exception const& e)
			{ eh.error_detected(error_message{e.what()}); }
			catch(char const* msg)
			{ eh.error_detected(error_message{msg}); }
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
