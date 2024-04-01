#ifndef TERRAFORMER_UI_DRAWING_API_SINGLE_QUAD_RENDERER_HPP
#define TERRAFORMER_UI_DRAWING_API_SINGLE_QUAD_RENDERER_HPP

#include "./gl_mesh.hpp"
#include "./gl_shader.hpp"

namespace terraformer::ui::drawing_api
{
	class single_quad_renderer
	{
	public:
		void render()
		{
			m_program.bind();
			m_mesh.bind();
			gl_bindings::draw_triangles();
		}

	private:
		gl_mesh<unsigned int, terraformer::location> m_mesh{
			std::array<unsigned int, 6>{
				0, 1, 2, 0, 2, 3
			},			
			std::array<terraformer::location, 4>{
				terraformer::location{-0.5f, -0.5f, 0.0f},
				terraformer::location{0.5f, -0.5f, 0.0f},
				terraformer::location{0.5f, 0.5f, 0.0f},
				terraformer::location{-0.5f, 0.5f, 0.0f},
			}
		};
		
		gl_program m_program{
			gl_shader<GL_VERTEX_SHADER>{
				R"(#version 460 core
layout (location = 0) in vec4 input_location;

out vec4 vertex_color;

void main()
{
	gl_Position = input_location;
	vertex_color = vec4(0.5, 0.5, 0.5, 1.0);
})"
			},
			gl_shader<GL_FRAGMENT_SHADER>{R"(#version 460 core
out vec4 fragment_color;
in vec4 vertex_color;

void main()
{
	fragment_color = vertex_color;
})"}
		};
	};
}

#endif
