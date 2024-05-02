#ifndef TERRAFORMER_UI_DRAWING_API_FRAME_RENDERER_HPP
#define TERRAFORMER_UI_DRAWING_API_FRAME_RENDERER_HPP

#include "./gl_mesh.hpp"
#include "./gl_shader.hpp"
#include "./gl_texture.hpp"
#include "ui/wsapi/native_window.hpp"

#include <cassert>

namespace terraformer::ui::drawing_api
{
	class frame_renderer
	{
	public:
		struct frame
		{
			gl_texture const* texture;
			float thickness;
			std::array<rgba_pixel, 4> inner_tints;
			std::array<rgba_pixel, 4> outer_tints;
		};

		void set_world_transform(location where, wsapi::fb_size size)
		{
			scaling const s{2.0f/static_cast<float>(size.width), 2.0f/static_cast<float>(size.height), 1.0f};
			m_program.set_uniform(3, where[0], where[1], where[2], 1.0f)
				.set_uniform(4, s[0], s[1], s[2], 0.0f);
		}

		void render(
			location where,
			location origin,
			scaling scale,
			struct frame const& frame
		)
		{
			auto const v = 0.5f*origin.get();
			m_program.set_uniform(0, where[0], where[1], where[2], 1.0f)
				.set_uniform(1, v[0], v[1], v[2], 1.0f)
				.set_uniform(2, scale[0], scale[1], scale[2], 0.0f)
				.set_uniform(5, frame.thickness)
				.set_uniform(6, frame.inner_tints)
				.bind();

			assert(frame.texture != nullptr);
			frame.texture->bind(0);
			m_mesh.bind();
			gl_bindings::draw_triangles();
		}

		auto& clear_buffers()
		{
			glClear(GL_COLOR_BUFFER_BIT);
			return *this;
		}

		auto& set_viewport(int x, int y, int width, int height)
		{
			glViewport(x, y, width, height);
			return *this;
		}

	private:
		gl_mesh<unsigned int> m_mesh{
			std::array<unsigned int, 6>{
				0, 1, 2, 0, 2, 3
			}
		};

		gl_program m_program{
			gl_shader<GL_VERTEX_SHADER>{
				R"(#version 460 core
layout (location = 0) uniform vec4 model_location;
layout (location = 1) uniform vec4 model_origin;
layout (location = 2) uniform vec4 model_size;
layout (location = 3) uniform vec4 world_location;
layout (location = 4) uniform vec4 world_scale;
layout (location = 5) uniform vec4 thickness;
layout (location = 6) uniform vec4 tints[4];

out vec2 uv;
out vec4 tints;

const vec2 uv_coords[4] = vec2[4](
	vec2(0.0f, 1.0f),
	vec2(1.0f, 1.0f),
	vec2(1.0f, 0.0f),
	vec2(0.0f, 0.0f)
);

const vec2 uv_offsets[4]=vec2[4](
	vec2(1.0f, 0.0f),
	vec2(-1.0f, 0.0f),
	vec2(0.0f),
	vec2(0.0f)
);

const vec4 coords[4] = vec4[4](
	vec4(-0.5f, -0.5f, 0.0f, 1.0f),
	vec4(0.5f, -0.5f, 0.0f, 1.0f),
	vec4(0.5f, 0.5f, 0.0f, 1.0f),
	vec4(-0.5,0.5f, 0.0f, 1.0f)
);

const vec4 vertex_offsets[4](
	vec4(1.0f, 0.0f, 0.0f, 0.0f),
	vec4(-1.0f, 0.0f, 0.0f, 0.0f),
	vec4(0.0f, 0.0f, 0.0f, 0.0f),
	vec4(0.0f, 0.0f, 0.0f, 0.0f)
);

void main()
{
	const vec4 world_origin = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	vec4 loc = model_location + model_size*(coords[gl_VertexID] - model_origin) + thickness*vertex_offsets[gl_VertexID];
	gl_Position = world_location + world_scale*(loc - world_origin);
	uv = model_size.xy*uv_coords[gl_VertexID] + thickness*vertex_offsets[gl_VertexID];
	tint = tints[gl_VertexID];
})"
			},
			gl_shader<GL_FRAGMENT_SHADER>{R"(#version 460 core
out vec4 fragment_color;
layout (binding = 0) uniform sampler2D background;

in vec2 uv;
in vec4 tint;

void main()
{
	vec4 color = texture(background, uv/textureSize(background, 0))*tint;
	fragment_color = color;
})"}
		};
	};
}

#endif
