#ifndef TERRAFORMER_UI_DRAWING_API_SINGLE_QUAD_RENDERER_HPP
#define TERRAFORMER_UI_DRAWING_API_SINGLE_QUAD_RENDERER_HPP

#include "./gl_mesh.hpp"
#include "./gl_shader.hpp"
#include "./gl_texture.hpp"
#include "ui/main/events.hpp"
#include "ui/main/widget_rendering_result.hpp"

#include "lib/pixel_store/image.hpp"

#include <cassert>

namespace terraformer::ui::drawing_api
{
	class single_quad_renderer
	{
	public:
		struct layer
		{
			using texture_type = gl_texture;
			texture_type const* texture;
			std::array<rgba_pixel, 4> tints;
		};

		struct layer_with_offset:layer
		{
			displacement offset;
		};

		struct input_rectangle
		{
			using texture_type = gl_texture;

			auto set_widget_background(texture_type const* texture, std::array<rgba_pixel, 4> const& tints)
			{
				if(texture == nullptr)
				{ return main::set_texture_result::incompatible; }

				widget_background.texture = texture;
				widget_background.tints = tints;
				return main::set_texture_result::success;
			}

			auto set_bg_layer_mask(texture_type const* texture)
			{
				if(texture == nullptr)
				{ return main::set_texture_result::incompatible; }

				bg_layer_mask = texture;
				return main::set_texture_result::success;
			}

			auto set_selection_background(texture_type const* texture, std::array<rgba_pixel, 4> const& tints)
			{
				if(texture == nullptr)
				{ return main::set_texture_result::incompatible; }

				selection_background.texture = texture;
				selection_background.tints = tints;
				return main::set_texture_result::success;
			}

			auto set_widget_foreground(texture_type const* texture, std::array<rgba_pixel, 4> const& tints)
			{
				if(texture == nullptr)
				{ return main::set_texture_result::incompatible; }

				widget_foreground.texture = texture;
				widget_foreground.tints = tints;
				return main::set_texture_result::success;
			}

			auto set_frame(texture_type const* texture, std::array<rgba_pixel, 4> const& tints)
			{
				if(texture == nullptr)
				{ return main::set_texture_result::incompatible; }

				frame.texture = texture;
				frame.tints = tints;
				return main::set_texture_result::success;
			}

			layer widget_background;
			texture_type const* bg_layer_mask;
			layer selection_background;
			layer_with_offset widget_foreground;
			layer frame;

			static gl_texture create_texture()
			{ return gl_texture{}; }
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
				.set_uniform(5, rect.widget_background.tints)
				.set_uniform(9, rect.selection_background.tints)
				.set_uniform(13, rect.widget_foreground.tints)
				.set_uniform(21, rect.widget_foreground.offset[0], rect.widget_foreground.offset[1])
				.set_uniform(17, rect.frame.tints)
				.bind();

			assert(rect.widget_background.texture != nullptr);
			assert(rect.bg_layer_mask != nullptr);
			assert(rect.selection_background.texture != nullptr);
			assert(rect.widget_foreground.texture != nullptr);
			assert(rect.frame.texture != nullptr);

			rect.widget_background.texture->bind(0);
			rect.bg_layer_mask->bind(1);
			rect.selection_background.texture->bind(2);
			rect.widget_foreground.texture->bind(3);
			rect.frame.texture->bind(4);

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
layout (location = 5) uniform vec4 widget_background_tints[4];
layout (location = 9) uniform vec4 selection_background_tints[4];
layout (location = 13) uniform vec4 widget_foreground_tints[4];
layout (location = 17) uniform vec4 frame_tints[4];

out vec2 uv;
out vec4 widget_background_tint;
out vec4 selection_background_tint;
out vec4 widget_foreground_tint;
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
	widget_background_tint = widget_background_tints[gl_VertexID];
	selection_background_tint = selection_background_tints[gl_VertexID];
	widget_foreground_tint = widget_foreground_tints[gl_VertexID];
	frame_tint = frame_tints[gl_VertexID];
})"
			},
			gl_shader<GL_FRAGMENT_SHADER>{R"(#version 460 core
out vec4 fragment_color;
layout (binding = 0) uniform sampler2D widget_background;
layout (binding = 1) uniform sampler2D bg_layer_mask;
layout (binding = 2) uniform sampler2D selection_background;
layout (binding = 3) uniform sampler2D widget_foreground;
layout (binding = 4) uniform sampler2D frame;
layout (location = 21) uniform vec2 fg_offset;

in vec2 uv;
in vec4 widget_background_tint;
in vec4 selection_background_tint;
in vec4 widget_foreground_tint;
in vec4 frame_tint;

vec4 sample_scaled(sampler2D tex, vec2 uv)
{
	return texture(tex, uv/textureSize(tex, 0));
}

vec4 sample_cropped(sampler2D tex, vec2 uv)
{
	vec2 uv_out = uv/textureSize(tex, 0);
	if(uv_out.x >= 0.0 && uv_out.x <= 1.0 && uv_out.y >= 0.0 && uv_out.y<=1.0)
	{ return texture(tex, uv_out); }
	else
	{ return vec4(0.0, 0.0, 0.0, 0.0); }
}

void main()
{
	vec4 bg_0 = sample_scaled(widget_background, uv)*widget_background_tint;
	float bg_mask = sample_scaled(bg_layer_mask, uv).r;
	vec4 bg_1 = sample_scaled(selection_background, uv)*selection_background_tint;
	vec4 fg_0 = sample_cropped(widget_foreground, uv - fg_offset)*widget_foreground_tint;
	vec4 fg_1 = texture(frame, uv)*frame_tint;

	// This assumes that pre-multiplied alpha is used
	vec4 result = bg_1 + bg_0*(1 - bg_1.w*bg_mask);
	result = fg_0 + result*(1 - fg_0.w);
	result = fg_1 + result*(1 - fg_1.w);

	fragment_color = result;
})"}
		};
	};
}

#endif
