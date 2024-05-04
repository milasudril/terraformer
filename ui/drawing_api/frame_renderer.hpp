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
		struct input_rectangle
		{
			gl_texture const* texture;
			std::array<rgba_pixel, 8> tints;
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
			input_rectangle const& rect
		)
		{
			auto const v = 0.5f*origin.get();
			m_program.set_uniform(0, where[0], where[1], where[2], 1.0f)
				.set_uniform(1, v[0], v[1], v[2], 1.0f)
				.set_uniform(2, scale[0], scale[1], scale[2], 0.0f)
				.set_uniform(5, rect.tints)
				.bind();

			assert(rect.texture != nullptr);
			rect.texture -> bind(0);

			m_mesh.bind();
			gl_bindings::draw_triangles_repeatedly(2);
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
			std::array<unsigned int, 12>{
				0, 1, 2, 2, 1, 3,
				4, 6, 5, 5, 6, 7
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
layout (location = 5) uniform vec4 tints[8];

out vec2 uv;
out vec4 tint;

const vec2 uv_coords[4] = vec2[4](
	vec2(0.0f, 0.0f),
	vec2(0.0f, 1.0f),
	vec2(1.0f, 0.0f),
	vec2(1.0f, 1.0f)
);

const vec4 coords[8] = vec4[8](
	// Top
	vec4(-0.5f, 0.5f, 0.0f, 1.0f),
	vec4(-0.5f, 0.5f, 0.0f, 1.0f),
	vec4(0.5f, 0.5f, 0.0f, 1.0f),
	vec4(0.5f, 0.5f, 0.0f, 1.0f),

	// Left
	vec4(-0.5f, 0.5f, 0.0f, 1.0f),
	vec4(-0.5f, 0.5f, 0.0f, 1.0f),
	vec4(-0.5f, -0.5f, 0.0f, 1.0f),
	vec4(-0.5f, -0.5f, 0.0f, 1.0f)
);

const vec4 offsets[8] = vec4[8](
	// Top
	vec4(0.0f, 0.0f, 0.0f, 0.0f),
	vec4(1.0f, -1.0f, 0.0f, 0.0f),
	vec4(0.0f, 0.0f, 0.0f, 0.0f),
	vec4(-1.0f, -1.0f, 0.0f, 0.0f),

	// Left
	vec4(0.0f, 0.0f, 0.0f, 0.0f),
	vec4(1.0f, -1.0f, 0.0f, 0.0f),
	vec4(0.0f, 0.0f, 0.0f, 0.0f),
	vec4(1.0f, 1.0f, 0.0f, 0.0f)
);

const int tint_map[16] = int[16](
	0, 1, 2, 3,
	0, 1, 4, 5,
	6, 7, 4, 5,
	6, 7, 2, 3
);

void main()
{
	const vec4 world_origin = vec4(0.0, 0.0, 0.0, 1.0);
	const float thickness = 200.0f;
	const float sign = ((gl_InstanceID&0x1) == 0x1)? -1.0f : 1.0f;
	vec4 loc = model_location
		+ model_size*(sign*coords[gl_VertexID] - model_origin)
		+ sign*thickness*offsets[gl_VertexID];
	gl_Position = world_location + world_scale*(loc - world_origin);
	uv = model_size.xy*uv_coords[gl_VertexID&0x3];
	const int tint_index = 8*gl_InstanceID + gl_VertexID;
	tint = tints[tint_map[tint_index]];
})"
			},
			gl_shader<GL_FRAGMENT_SHADER>{R"(#version 460 core
out vec4 fragment_color;
layout (binding = 0) uniform sampler2D background;
layout (binding = 1) uniform sampler2D foreground;
layout (location = 13) uniform vec4 fg_bg_separator[2];

in vec2 uv;
in vec4 tint;

void main()
{
	//vec4 bg = texture(background, uv/textureSize(background, 0))*background_tint;
	//vec4 fg = texture(foreground, uv/textureSize(foreground, 0))*foreground_tint;

	fragment_color = tint;
})"}
		};
	};
}

#endif
