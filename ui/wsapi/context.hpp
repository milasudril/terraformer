//@	{
//@	 "dependencies":[{"ref": "glfw3", "origin":"pkg-config"}]
//@	}

#ifndef TERRAFORMER_UI_WSAPI_CONTEXT_HPP
#define TERRAFORMER_UI_WSAPI_CONTEXT_HPP

#define GLFW_INCLUDE_NONE

#include "lib/pixel_store/rgba_pixel.hpp"
#include "lib/common/span_2d.hpp"

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include <memory>

namespace terraformer::ui::wsapi
{
	struct cursor_deleter
	{
		void operator()(GLFWcursor* cursor) const
		{ glfwDestroyCursor(cursor); }
	};

	using cursor_handle = std::unique_ptr<GLFWcursor, cursor_deleter>;

	class cursor
	{
	public:
		struct pixel_type
		{
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t a;
		};

		explicit cursor(
			span_2d<pixel_type> pixels,
			int x_hot,
			int y_hot
		)
		{
			GLFWimage const img{
				.width = static_cast<int>(pixels.width()),
				.height = static_cast<int>(pixels.height()),
				.pixels = reinterpret_cast<unsigned char*>(pixels.data())
			};

			m_handle = cursor_handle{glfwCreateCursor(&img, x_hot, y_hot)};
		}

		auto handle() const
		{ return m_handle.get(); }

	private:
		cursor_handle m_handle;
	};

	class context
	{
	public:
		using cursor_pixel_type = cursor::pixel_type;

		[[nodiscard]] static constexpr auto make_cursor_pixel(rgba_pixel value)
		{
			return cursor_pixel_type{
				static_cast<uint8_t>(255.0f*std::pow(value.red(), 1.0f/2.2f)),
				static_cast<uint8_t>(255.0f*std::pow(value.green(), 1.0f/2.2f)),
				static_cast<uint8_t>(255.0f*std::pow(value.blue(), 1.0f/2.2f)),
				static_cast<uint8_t>(255.0f*std::pow(value.alpha(), 1.0f/2.2f))
			};
		}

		[[nodiscard]] static auto create_cursor(
			span_2d<cursor_pixel_type> pixels,
			int x_hot,
			int y_hot
		)
		{ return cursor{pixels, x_hot, y_hot}; }

		[[nodiscard]] static context& get_instance()
		{
			static context ctxt;
			return ctxt;
		}

		template<class Callable, class ... Args>
		void poll_events(Callable&& f, Args&&... args)
		{
			while(true)
			{
				glfwPollEvents();
				if(f(args...))
				{ return; }
			}
		}

		template<class Callable, class ... Args>
		void wait_events(Callable&& f, Args&&... args)
		{
			while(true)
			{
				glfwWaitEvents();
				if(f(args...))
				{ return; }
			}
		}

		void notify_main_loop()
		{ glfwPostEmptyEvent(); }

	private:
		context()
		{ glfwInit(); }

		~context()
		{ glfwTerminate(); }
	};
}

#endif