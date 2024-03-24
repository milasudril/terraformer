//@	{
//@	 "dependencies":[{"ref": "glfw3", "origin":"pkg-config"}]
//@	}

#ifndef TERRAFORMER_UI_WSAPI_CONTEXT_HPP
#define TERRAFORMER_UI_WSAPI_CONTEXT_HPP

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <GL/gl.h>

namespace terraformer::ui::wsapi
{
	class context
	{
	public:
		static context& get()
		{
			static context ctxt;
			return ctxt;
		}

		template<class Callable, class ... Args>
		void read_events(Callable&& f, Args&&... args)
		{
			while(true)
			{
				glfwPollEvents();
				if(f(args...))
				{ return; }
			}
		}

	private:
		context()
		{
			glfwInit();
		}

		~context()
		{
			glfwTerminate();
		}
	};
}

#endif