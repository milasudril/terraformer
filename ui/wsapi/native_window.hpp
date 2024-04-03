#ifndef TERRAFORMER_UI_WSAPI_HPP
#define TERRAFORMER_UI_WSAPI_HPP

#include "./context.hpp"
#include "./events.hpp"

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include <memory>
#include <stdexcept>
#include <cstdio>
#include <optional>

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

	constexpr window_features operator~(window_features value)
	{ return static_cast<window_features>(~static_cast<uint32_t>(value)); }

	constexpr window_features operator|(window_features a, window_features b)
	{ return static_cast<window_features>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }

	constexpr window_features& operator|=(window_features& a, window_features b)
	{ return a = a | b; }

	constexpr window_features operator&(window_features a, window_features b)
	{ return static_cast<window_features>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b)); }

	constexpr window_features& operator&=(window_features& a, window_features b)
	{ return a = a & b; }

	constexpr window_features operator^(window_features a, window_features b)
	{ return static_cast<window_features>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b)); }

	constexpr auto is_set(window_features a, window_features feature)
	{ return static_cast<bool>(a & feature); }

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
			glfwSetWindowUserPointer(m_window.get(), &eh.get());
			if constexpr (requires{{eh.get().window_is_closing()}->std::same_as<void>;})
			{
				glfwSetWindowCloseCallback(m_window.get(), [](GLFWwindow* window) -> void {
					auto eh = static_cast<EventHandler*>(glfwGetWindowUserPointer(window));
					eh->window_is_closing();
				});
			}

			if constexpr (requires(fb_size size){{eh.get().framebuffer_size_changed(size)}->std::same_as<void>;})
			{
				glfwSetFramebufferSizeCallback(
					m_window.get(),
					[](GLFWwindow* window, int w, int h){
						auto eh = static_cast<EventHandler*>(glfwGetWindowUserPointer(window));
						eh->framebuffer_size_changed(
							fb_size{
								.width = w,
								.height = h
							}
						);
					}
				);
				eh.get().framebuffer_size_changed(get_fb_size());
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
	};
}

#endif
