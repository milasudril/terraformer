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

		struct input_rectangle
		{
			using texture_type = gl_texture;

			auto set_background(texture_type const* texture)
			{
				if(texture == nullptr)
				{ return main::set_texture_result::incompatible; }

				background.texture = texture;
				return main::set_texture_result::success;
			}

			auto set_foreground(texture_type const* texture)
			{
				if(texture == nullptr)
				{ return main::set_texture_result::incompatible; }

				foreground.texture = texture;
				return main::set_texture_result::success;
			}

			void set_background_tints(std::array<rgba_pixel, 4> const& vals)
			{ background.tints = vals; }

			void set_foreground_tints(std::array<rgba_pixel, 4> const& vals)
			{ foreground.tints = vals; }

			layer background;
			layer foreground;

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
				.set_uniform(5, rect.background.tints)
				.set_uniform(9, rect.foreground.tints)
				.bind();

			assert(rect.background.texture != nullptr);
			assert(rect.foreground.texture != nullptr);

			rect.background.texture->bind(0);
			rect.foreground.texture->bind(1);

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
layout (location = 9) uniform vec4 foreground_tints[4];

out vec2 uv;
out vec4 background_tint;
out vec4 foreground_tint;

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
	foreground_tint = foreground_tints[gl_VertexID];
})"
			},
			gl_shader<GL_FRAGMENT_SHADER>{R"(#version 460 core
out vec4 fragment_color;
layout (binding = 0) uniform sampler2D background;
layout (binding = 1) uniform sampler2D foreground;

in vec2 uv;
in vec4 background_tint;
in vec4 foreground_tint;

void main()
{
	vec4 bg = texture(background, uv/textureSize(background, 0))*background_tint;
	vec4 fg = texture(foreground, uv/textureSize(foreground, 0))*foreground_tint;

	// This assumes values are pre-multiplied alphaś
	fragment_color = fg + bg*(1 - fg.w);
})"}
		};
	};
}

#endif
