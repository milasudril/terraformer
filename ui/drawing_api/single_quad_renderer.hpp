#ifndef TERRAFORMER_UI_DRAWING_API_SINGLE_QUAD_RENDERER_HPP
#define TERRAFORMER_UI_DRAWING_API_SINGLE_QUAD_RENDERER_HPP

#include "./gl_mesh.hpp"
#include "./gl_shader.hpp"
#include "./gl_texture.hpp"
#include "ui/wsapi/native_window.hpp"
#include "ui/main/default_stock_textures_repo.hpp"
#include "lib/common/value_accessor.hpp"

#include <variant>

namespace terraformer::ui::drawing_api
{
	class single_quad_renderer
	{
	public:
		struct fg_bg_separator
		{
			location begin;
			location end;
		};

		struct input_rectangle
		{
			gl_texture const* background;
			gl_texture const* foreground;
			std::array<rgba_pixel, 4> background_tints;
			std::array<rgba_pixel, 4> foreground_tints;
			struct fg_bg_separator fg_bg_separator;
		};

		[[nodiscard]] static single_quad_renderer& get_default_instance()
		{
			thread_local single_quad_renderer ret{};
			return ret;
		}

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
				.set_uniform(5, rect.background_tints)
				.set_uniform(9, rect.foreground_tints)
				.bind();

			assert(rect.background != nullptr);
			rect.background -> bind(0);

			assert(rect.foreground != nullptr);
			rect.foreground -> bind(1);

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
layout (location = 13) uniform vec4 fg_bg_separator[2];

in vec2 uv;
in vec4 background_tint;
in vec4 foreground_tint;

void main()
{
	vec4 bg = texture(background, uv/textureSize(background, 0))*background_tint;
	vec4 fg = texture(foreground, uv/textureSize(foreground, 0))*foreground_tint;

	// This assumes values are pre-multiplied alpha. Otherwise, the formula for the color components
	// would be
	//
	// (C_fg*a_fg + C_bg*a_bg*(1 - a_fg))/(a_fg + a_bg*(1 - a_fg))
	//
	// If a_bg = 1, this simplifies to
	//
	// (C_fg*a_fg + C_bg*(1 - a_fg))/(a_fg + (1 - a_fg)) = C_fg*a_fg + C_bg*(1 - a_fg)
	//
	// which is the regular interpolation formula, with a_fg as interpolation parameter.
	//
	fragment_color = mix(bg, fg, fg.w);
})"}
		};
	};
}

#endif
