//@	{
//@	 "dependencies":[{"ref": "glfw3", "origin":"pkg-config"},
//@		{"ref": "glew", "origin":"pkg-config"}]
//@	}

#ifndef TERRAFORMER_GLVIEWPORT_HPP
#define TERRAFORMER_GLVIEWPORT_HPP

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include <memory>
#include <stdexcept>
#include <cstdio>

namespace terraformer
{
	struct window_handle_deleter
	{
		void operator()(GLFWwindow* window) const
		{
			if(window != nullptr)
			{
				glfwDestroyWindow(window);
			}
		}
	};

	using window_handle = std::unique_ptr<GLFWwindow, window_handle_deleter>;

	class glfw_context
	{
	public:
		static glfw_context& get()
		{
			static glfw_context ctxt;
			return ctxt;
		}

		template<class Callable, class ... Args>
		void read_events(Callable&& f, Args&&... args)
		{
			while(true)
			{
				glfwPollEvents();
				if(!f(args...))
				{ return; }
			}
		}

	private:
		glfw_context()
		{
			glfwInit();
		}

		~glfw_context()
		{
			glfwTerminate();
		}
	};

	class gl_viewport
	{
	public:
		explicit gl_viewport(glfw_context const&,
			uint32_t width, uint32_t height, char const* title)
		{
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
			glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
			glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
			m_window.reset(glfwCreateWindow(width, height, title, nullptr, nullptr));
			if(m_window == nullptr)
			{ throw std::runtime_error{"Failed to create a window"}; }

			activate_gl_context();
			fprintf(stderr, "(i) Initialized OpenGL reporting version %s\n", glGetString(GL_VERSION));
		}

		void activate_gl_context()
		{
			glfwMakeContextCurrent(m_window.get());
			static bool glew_initialized = false;
			if(glew_initialized == false)
			{
				GLenum err = glewInit();
				if (GLEW_OK != err)
				{ throw std::runtime_error{"Failed to load OpenGL extensions"}; }
				glew_initialized = true;
			}
			glfwSwapInterval(1);
		}

		void swap_buffers()
		{ glfwSwapBuffers(m_window.get()); }

		template<class EventHandler>
		void set_event_handler(EventHandler& eh)
		{
			glfwSetWindowUserPointer(m_window.get(), &eh);
			glfwSetWindowCloseCallback(m_window.get(), [](GLFWwindow* window){
				auto eh = static_cast<EventHandler*>(glfwGetWindowUserPointer(window));
				eh->window_is_closing();
			});
			glfwSetFramebufferSizeCallback(m_window.get(), [](GLFWwindow* window, int w, int h){
				auto eh = static_cast<EventHandler*>(glfwGetWindowUserPointer(window));
				eh->framebuffer_size_changed(w, h);
			});
		}

		void set_window_title(char const* title)
		{	glfwSetWindowTitle(m_window.get(), title); }

	private:
		window_handle m_window;
	};
}

#endif
