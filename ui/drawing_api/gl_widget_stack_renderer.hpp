#ifndef TERRAFORMER_UI_DRAWING_API_WIDGET_LAYER_STACK_RENDERER_HPP
#define TERRAFORMER_UI_DRAWING_API_WIDGET_LAYER_STACK_RENDERER_HPP

#include "./gl_mesh.hpp"
#include "./gl_shader.hpp"
#include "ui/main/widget_layer_stack.hpp"

#include "lib/pixel_store/image.hpp"

#include <cassert>

namespace terraformer::ui::drawing_api
{
	class gl_widget_layer_stack_renderer
	{
	public:
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
			main::widget_layer_stack const& rect
		)
		{
			auto const v = 0.5f*origin.get();
			m_program.set_uniform(0, where[0], where[1], where[2], 1.0f)
				.set_uniform(1, v[0], v[1], v[2], 1.0f)
				.set_uniform(2, scale[0], scale[1], scale[2], 0.0f)
				.set_uniform(5, rect.background.tints)
				.set_uniform(9, rect.selection_background.tints)
				.set_uniform(13, rect.foreground.tints)
				.set_uniform(17, rect.input_marker.tints)
				.set_uniform(21, rect.frame.tints)
				.set_uniform(25, rect.foreground.offset[0], rect.foreground.offset[1])
				.set_uniform(26, rect.input_marker.offset[0], rect.input_marker.offset[1])
				.set_uniform(27, static_cast<float>(to_rad(rect.foreground.rotation).value))
				.bind();

			assert(rect.background.texture);
			assert(rect.sel_bg_mask.texture);
			assert(rect.selection_background.texture);
			assert(rect.foreground.texture);
			assert(rect.input_marker.texture);
			assert(rect.frame.texture);

			rect.background.texture.bind(0);
			rect.sel_bg_mask.texture.bind(1);
			rect.selection_background.texture.bind(2);
			rect.foreground.texture.bind(3);
			rect.input_marker.texture.bind(4);
			rect.frame.texture.bind(5);

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
layout (location = 5) uniform vec4 background_tints[4];
layout (location = 9) uniform vec4 selection_background_tints[4];
layout (location = 13) uniform vec4 foreground_tints[4];
layout (location = 17) uniform vec4 input_marker_tints[4];
layout (location = 21) uniform vec4 frame_tints[4];

out vec2 uv;
out vec4 background_tint;
out vec4 selection_background_tint;
out vec4 foreground_tint;
out vec4 input_marker_tint;
out vec4 frame_tint;

const vec2 uv_coords[4] = vec2[4](
	vec2(0.0f, 1.0f),
	vec2(1.0f, 1.0f),
	vec2(1.0f, 0.0f),
	vec2(0.0f, 0.0f)
);

const vec4 coords[4] = vec4[4](
	vec4(-0.5f, -0.5, 0.0, 1.0f),
	vec4(0.5, -0.5, 0.0, 1.0f),
	vec4(0.5, 0.5, 0.0, 1.0f),
	vec4(-0.5,0.5, 0.0, 1.0f)
);

void main()
{
	const vec4 world_origin = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 loc = model_location + model_size*(coords[gl_VertexID] - model_origin);
	gl_Position = world_location + world_scale*(loc - world_origin);
	uv = model_size.xy*uv_coords[gl_VertexID];
	background_tint = background_tints[gl_VertexID];
	selection_background_tint = selection_background_tints[gl_VertexID];
	foreground_tint = foreground_tints[gl_VertexID];
	input_marker_tint = input_marker_tints[gl_VertexID];
	frame_tint = frame_tints[gl_VertexID];
})"
			},
			gl_shader<GL_FRAGMENT_SHADER>{R"(#version 460 core
out vec4 fragment_color;
layout (binding = 0) uniform sampler2D background;
layout (binding = 1) uniform sampler2D bg_layer_mask;
layout (binding = 2) uniform sampler2D selection_background;
layout (binding = 3) uniform sampler2D foreground;
layout (binding = 4) uniform sampler2D input_marker;
layout (binding = 5) uniform sampler2D frame;
layout (location = 25) uniform vec2 fg_offset;
layout (location = 26) uniform vec2 input_marker_offset;
layout (location = 27) uniform float fg_0_rotation;

in vec2 uv;
in vec4 background_tint;
in vec4 selection_background_tint;
in vec4 foreground_tint;
in vec4 input_marker_tint;
in vec4 frame_tint;

vec4 sample_scaled(sampler2D tex, vec2 uv)
{
	return texture(tex, uv/textureSize(tex, 0));
}

vec4 sample_cropped(sampler2D tex, vec2 uv, float rot_angle)
{
	vec2 uv_scaled = uv/textureSize(tex, 0);
	vec2 uv_centered = uv_scaled - vec2(0.5, 0.5);
	uv_centered = vec2(
		dot(uv_centered, vec2(cos(rot_angle), sin(rot_angle))),
		dot(uv_centered, vec2(-sin(rot_angle), cos(rot_angle)))
	);
	vec2 uv_out = uv_centered + vec2(0.5, 0.5);
	// FIXME: Need to crop based on view window as well
	if(uv_out.x >= 0.0 && uv_out.x <= 1.0 && uv_out.y >= 0.0 && uv_out.y<=1.0)
	{ return texture(tex, uv_out); }
	else
	{ return vec4(0.0, 0.0, 0.0, 0.0); }
}

void main()
{
	vec4 bg_0 = sample_scaled(background, uv)*background_tint;
	float bg_mask = sample_cropped(bg_layer_mask, uv - fg_offset, 0.0).r;
	vec4 bg_1 = sample_scaled(selection_background, uv)*selection_background_tint;
	vec4 fg_0 = sample_cropped(foreground, uv - fg_offset, fg_0_rotation)*foreground_tint;
	vec4 fg_1 = sample_cropped(input_marker, uv - input_marker_offset, 0.0)*input_marker_tint;
	vec4 fg_2 = sample_scaled(frame, uv)*frame_tint;

	// This assumes that pre-multiplied alpha is used
	vec4 result = bg_1 + bg_0*(1 - bg_1.w*bg_mask);
	result = fg_0 + result*(1 - fg_0.w);
	result = fg_1 + result*(1 - fg_1.w);
	result = fg_2 + result*(1 - fg_2.w);

	fragment_color = result;
})"}
		};
	};
}

#endif
