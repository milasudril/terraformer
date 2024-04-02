#ifndef TERRAFORMER_UI_DRAWING_API_SINGLE_QUAD_RENDERER_HPP
#define TERRAFORMER_UI_DRAWING_API_SINGLE_QUAD_RENDERER_HPP

#include "./gl_mesh.hpp"
#include "./gl_shader.hpp"
#include "ui/wsapi/native_window.hpp"

namespace terraformer::ui::drawing_api
{
	class single_quad_renderer
	{
	public:
		void set_world_transform(location where, wsapi::fb_size size)
		{
			scaling const s{1.0f/static_cast<float>(size.width), 1.0f/static_cast<float>(size.height), 1.0f};
			m_program.bind();
			glUniform4f(3, where[0], where[1], where[2], 1.0f);
			glUniform4f(4, s[0], s[1], s[2], 0.0f);
		}

		void render(location where, location origin, scaling scale)
		{
			m_program.bind();
			m_mesh.bind();
			glUniform4f(0, where[0], where[1], where[2], 1.0f);
			glUniform4f(1, origin[0], origin[1], origin[2], 1.0f);
			glUniform4f(2, scale[0], scale[1], scale[2], 0.0f);
			gl_bindings::draw_triangles();
		}

	private:
		gl_mesh<unsigned int, location> m_mesh{
			std::array<unsigned int, 6>{
				0, 1, 2, 0, 2, 3
			},
			std::array<location, 4>{
				location{-1.0f, -1.0f, 0.0f},
				location{1.0f, -1.0f, 0.0f},
				location{1.0f, 1.0f, 0.0f},
				location{-1.0f,1.0f, 0.0f},
			}
		};

		gl_program m_program{
			gl_shader<GL_VERTEX_SHADER>{
				R"(#version 460 core
layout (location = 0) in vec4 input_offset;

layout (location = 0) uniform vec4 model_location;
layout (location = 1) uniform vec4 model_origin;
layout (location = 2) uniform vec4 model_size;
layout (location = 3) uniform vec4 world_location;
layout (location = 4) uniform vec4 world_scale;

out vec4 vertex_color;

void main()
{
	const vec4 world_origin = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 loc = model_location + model_size*(input_offset - model_origin);
	gl_Position = world_location + world_scale*(loc - world_origin);

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
