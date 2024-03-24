//@	{
//@	 "dependencies":[
//@			{"ref": "glew", "origin":"pkg-config"}
//@		]
//@	}

#ifndef TERRAFORMER_UI_RENDERER_GL_SURFACE_CONFIGURATION_HPP
#define TERRAFORMER_UI_RENDERER_GL_SURFACE_CONFIGURATION_HPP

#include "ui/wsapi/context.hpp"

#include <GL/glew.h>
#include <GL/gl.h>
#include <cstdio>
#include <stdexcept>

namespace terraformer::ui::renderer
{
	struct drawing_api_version
	{
		int major;
		int minor;
	};

	struct gl_surface_configuration
	{
		drawing_api_version api_version{3, 3};
		int depth_buffer_bits{GLFW_DONT_CARE};
		int multisampling{4};
		int buffer_swap_interval{1};
		bool use_srgb{GLFW_TRUE};
	};

	inline void prepare_surface(wsapi::context&, gl_surface_configuration const& cfg)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, cfg.api_version.major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, cfg.api_version.minor);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_DEPTH_BITS, cfg.depth_buffer_bits);
		glfwWindowHint(GLFW_SRGB_CAPABLE, cfg.use_srgb? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_SAMPLES, cfg.multisampling);
		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	}

	inline void activate_render_context(GLFWwindow* window, gl_surface_configuration const& cfg)
	{
		glfwMakeContextCurrent(window);
		static bool glew_initialized = false;
		if(glew_initialized == false)
		{
			GLenum err = glewInit();
			if (GLEW_OK != err)
			{ throw std::runtime_error{"Failed to load OpenGL extensions"}; }
			glew_initialized = true;
			glfwSwapInterval(cfg.buffer_swap_interval);
		}
		if(cfg.multisampling != 0)
		{ glEnable(GL_MULTISAMPLE); }
		fprintf(stderr, "(i) Initialized OpenGL reporting version %s\n", glGetString(GL_VERSION));
	}

	inline void swap_buffers(GLFWwindow* window, gl_surface_configuration const&)
	{ glfwSwapBuffers(window); }
}

#endif