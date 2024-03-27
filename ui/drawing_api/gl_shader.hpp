#ifndef TERRAFORMER_UI_DRAWING_API_GLSHADER_HPP
#define TERRAFORMER_UI_DRAWING_API_GLSHADER_HPP

#include "./gl_resource.hpp"

#include <memory>

namespace terraformer::ui::drawing_api
{
	struct gl_shader_deleter
	{
		void operator()(GLuint handle) const
		{
			glDeleteShader(handle);
		}
	};

	using gl_shader_handle = gl_resource<gl_shader_deleter>;

	template<GLenum Type>
	class gl_shader
	{
	public:
		explicit gl_shader(char const* src):m_handle{glCreateShader(Type)}
		{
			glShaderSource(m_handle.get(), 1, &src, nullptr);
			glCompileShader(m_handle.get());
		}

		auto get() const { return m_handle.get(); }

	private:
		gl_shader_handle m_handle;
	};

	struct gl_program_deleter
	{
		void operator()(GLuint handle) const
		{
			glDeleteProgram(handle);
		}
	};

	using gl_program_handle = gl_resource<gl_program_deleter>;

	class gl_program
	{
	public:
		explicit gl_program(gl_shader<GL_VERTEX_SHADER> const& vertex_shader,
			gl_shader<GL_FRAGMENT_SHADER> const& fragment_shader):
			m_handle{glCreateProgram()}
		{
			glAttachShader(m_handle.get(), vertex_shader.get());
			glAttachShader(m_handle.get(), fragment_shader.get());
			glLinkProgram(m_handle.get());
		}

		void bind() const
		{
			glUseProgram(m_handle.get());
		}

	private:
		gl_program_handle m_handle;
	};
}

#endif