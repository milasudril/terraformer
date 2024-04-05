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
			scaling const s{2.0f/static_cast<float>(size.width), 2.0f/static_cast<float>(size.height), 1.0f};
			m_program.set_uniform(3, where[0], where[1], where[2], 1.0f)
				.set_uniform(4, s[0], s[1], s[2], 0.0f);
		}

		void render(location where, location origin, scaling scale)
		{
			auto const v = 0.5f*origin.get();
			m_program.set_uniform(0, where[0], where[1], where[2], 1.0f)
				.set_uniform(1, v[0], v[1], v[2], 1.0f)
				.set_uniform(2, scale[0], scale[1], scale[2], 0.0f)
				.bind();

			m_mesh.bind();

			gl_bindings::draw_triangles();
		}

	private:
		gl_mesh<unsigned int, location> m_mesh{
			std::array<unsigned int, 6>{
				0, 1, 2, 0, 2, 3
			},
			std::array<location, 4>{
				location{-0.5f, -0.5f, 0.0f},
				location{0.5f, -0.5f, 0.0f},
				location{0.5f, 0.5f, 0.0f},
				location{-0.5f,0.5f, 0.0f},
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

out vec2 uv;
const vec2 uv_coords[4] = vec2[4](
	vec2(0.0f, 1.0f),
	vec2(1.0f, 1.0f),
	vec2(1.0f, 0.0f),
	vec2(0.0f, 0.0f)
);

void main()
{
	const vec4 world_origin = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 loc = model_location + model_size*(input_offset - model_origin);
	gl_Position = world_location + world_scale*(loc - world_origin);
	uv = uv_coords[gl_VertexID];
})"
			},
			gl_shader<GL_FRAGMENT_SHADER>{R"(#version 460 core
layout (location = 0) out vec4 fragment_color;
layout (binding  = 0) uniform sampler2D theTexture;

in vec2 uv;

void main()
{
	fragment_color = texture(theTexture, uv);
})"}
		};
	};
}

#endif
