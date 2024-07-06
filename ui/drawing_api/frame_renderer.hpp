#ifndef TERRAFORMER_UI_DRAWING_API_FRAME_RENDERER_HPP
#define TERRAFORMER_UI_DRAWING_API_FRAME_RENDERER_HPP

#include "./gl_mesh.hpp"
#include "./gl_shader.hpp"
#include "./gl_texture.hpp"
#include "ui/main/events.hpp"

#include <cassert>

namespace terraformer::ui::drawing_api
{
	class frame_renderer
	{
	public:
		struct input_rectangle
		{
			float thickness;
			gl_texture const* texture;
			std::array<rgba_pixel, 8> tints;
		};

		void set_world_transform(location where, main::fb_size size)
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
				.set_uniform(5, rect.thickness)
				.set_uniform(6, rect.tints)
				.bind();

			assert(rect.texture != nullptr);
			rect.texture->bind(0);

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
			std::array<unsigned int, 24>{
				0, 1, 2, 2, 1, 3,
				4, 5, 6, 6, 5, 7,
				8, 9, 11, 8, 11, 10,
				12, 13, 15, 12, 15, 14
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
layout (location = 5) uniform float thickness;
layout (location = 6) uniform vec4 tints[8];

layout (binding = 0) uniform sampler2D tex;

out vec2 uv;
out vec4 tint;

const vec4 coords[16] = vec4[16](
	// Top
	vec4(-0.5f, 0.5f, 0.0f, 1.0f),
	vec4(-0.5f, 0.5f, 0.0f, 1.0f),
	vec4(0.5f, 0.5f, 0.0f, 1.0f),
	vec4(0.5f, 0.5f, 0.0f, 1.0f),

	// Left
	vec4(-0.5f, -0.5f, 0.0f, 1.0f),
	vec4(-0.5f, -0.5f, 0.0f, 1.0f),
	vec4(-0.5f, 0.5f, 0.0f, 1.0f),
	vec4(-0.5f, 0.5f, 0.0f, 1.0f),

	// Bottom corner
	vec4(-0.5f, -0.5f, 0.0f, 1.0f),
	vec4(-0.5f, -0.5f, 0.0f, 1.0f),
	vec4(-0.5f, -0.5f, 0.0f, 1.0f),
	vec4(-0.5f, -0.5f, 0.0f, 1.0f),

	// Top corner
	vec4(-0.5f, 0.5f, 0.0f, 1.0f),
	vec4(-0.5f, 0.5f, 0.0f, 1.0f),
	vec4(-0.5f, 0.5f, 0.0f, 1.0f),
	vec4(-0.5f, 0.5f, 0.0f, 1.0f)
);

const vec4 offsets[16] = vec4[16](
	// Top
	vec4(1.0f, 0.0f, 0.0f, 0.0f),
	vec4(1.0f, -1.0f, 0.0f, 0.0f),
	vec4(-1.0f, 0.0f, 0.0f, 0.0f),
	vec4(-1.0f, -1.0f, 0.0f, 0.0f),

	// Left
	vec4(0.0f, 1.0f, 0.0f, 0.0f),
	vec4(1.0f, 1.0f, 0.0f, 0.0f),
	vec4(0.0f, -1.0f, 0.0f, 0.0f),
	vec4(1.0f, -1.0f, 0.0f, 0.0f),

	// Bottom corner
	vec4(0.0f, 0.0f, 0.0f, 1.0f),
	vec4(1.0f, 0.0f, 0.0f, 1.0f),
	vec4(0.0f, 1.0f, 0.0f, 1.0f),
	vec4(1.0f, 1.0f, 0.0f, 1.0f),

	// Top corner
	vec4(0.0f, 0.0f, 0.0f, 1.0f),
	vec4(0.0f, -1.0f, 0.0f, 1.0f),
	vec4(1.0f, 0.0f, 0.0f, 1.0f),
	vec4(1.0f, -1.0f, 0.0f, 1.0f)
);

const int tint_map[32] = int[32](
	// Top
	0, 4, 1, 5,

	// Left
	2, 6, 0, 4,

	// Bottom-left corner
	2, 2, 2, 6,

	// Top-left corner
	0, 0, 0, 4,

	// Bottom
	3, 7, 2, 6,

	// Right
	1, 5, 3, 7,

	// Top-right corner
	1, 1, 1, 5,

	// Bottom-right corner
	3, 3, 3, 7
);

const vec2 uv_coords[4] = vec2[4](
	vec2(0.0f, 0.0f),
	vec2(0.0f, 1.0f),
	vec2(1.0f, 0.0f),
	vec2(1.0f, 1.0f)
);

void main()
{
	const vec4 world_origin = vec4(0.0, 0.0, 0.0, 1.0);
	const float sign = ((gl_InstanceID&0x1) == 0x1)? -1.0f : 1.0f;
	vec4 loc = model_location
		+ model_size*(sign*coords[gl_VertexID] - model_origin)
		+ sign*thickness*offsets[gl_VertexID];
	gl_Position = world_location + world_scale*(loc - world_origin);

	const float length = (gl_VertexID < 4) ? model_size.x : model_size.y;
	const float seg_length = length - 2.0f*thickness;
	const vec2 tex_size = textureSize(tex, 0);
	const float texture_width = tex_size.x;
	const float texture_height = tex_size.y;
	const float island_width = 0.25f*(texture_width - 8)/texture_width;
	const float island_hight = 0.2f*(texture_height - 10)/texture_height;
	const float uv_horz_scale = 0.2f*texture_height*seg_length/(texture_width * thickness);
	const vec2 uv_size = vec2(
		(gl_VertexID < 8)? uv_horz_scale : thickness,
		island_hight
	);
	const vec2 uv_pitch = vec2((gl_VertexID < 8)? uv_horz_scale : thickness, 0.2f);
	const int quad_index = gl_VertexID/4;
	const int segment_index = 2*gl_InstanceID + quad_index;
	uv = (gl_VertexID < 8) ?
		vec2(-0.5f, float(segment_index))*uv_pitch + uv_coords[gl_VertexID&0x3]*uv_size + vec2(0.5f, 1.0f/texture_height)
		: vec2(0.25f*float(segment_index - 2) + 1.0f/texture_width, 0.8f + 1.0f/texture_height)
		+ vec2(island_width, island_hight)*uv_coords[gl_VertexID&0x3];

	const int tint_index = 16*gl_InstanceID + gl_VertexID;
	tint = tints[tint_map[tint_index]];
})"
			},
			gl_shader<GL_FRAGMENT_SHADER>{R"(#version 460 core
out vec4 fragment_color;
layout (binding = 0) uniform sampler2D tex;

in vec2 uv;
in vec4 tint;

void main()
{
	fragment_color = texture(tex, uv)*tint;
})"}
		};
	};
}

#endif
