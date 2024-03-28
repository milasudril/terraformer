//@	{
//@		"target":{"name":"terraformer-gui.o"}
//@	}

#include "./drawing_api/gl_surface_configuration.hpp"
#include "./drawing_api/gl_texture.hpp"
#include "./drawing_api/gl_mesh.hpp"
#include "./drawing_api/gl_shader.hpp"
#include "./main/event_dispatcher.hpp"
#include "./layout_handling/layout_controller_entry.hpp"
#include "./wsapi/native_window.hpp"

namespace
{
	struct my_event_handler
	{
		terraformer::ui::drawing_api::gl_mesh<unsigned int, terraformer::location> the_mesh{
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

		terraformer::ui::drawing_api::gl_program the_program{
			terraformer::ui::drawing_api::gl_shader<GL_VERTEX_SHADER>{
				R"(#version 460 core
layout (location = 0) in vec4 input_location;

out vec4 vertex_color;

void main()
{
	gl_Position = input_location;
	vertex_color = vec4(0.5, 0.5, 0.5, 1.0);
})"
			},
			terraformer::ui::drawing_api::gl_shader<GL_FRAGMENT_SHADER>{R"(#version 460 core
out vec4 fragment_color;
in vec4 vertex_color;

void main()
{
	fragment_color = vertex_color;
})"}
		};

		void window_is_closing()
		{ should_close = true; }

		void framebuffer_size_changed(terraformer::ui::wsapi::fb_size size)
		{fb_size = size;}

		bool operator()(
			terraformer::ui::wsapi::native_window<terraformer::ui::drawing_api::gl_surface_configuration>& viewport
		)
		{
			glClear(GL_COLOR_BUFFER_BIT);
			the_program.bind();
			the_mesh.bind();
			terraformer::ui::drawing_api::gl_bindings::draw_triangles();
			viewport.swap_buffers();
			return should_close;
		}

		bool should_close{false};
		terraformer::ui::wsapi::fb_size fb_size;
	};
}

int main(int, char**)
{
	auto& gui_ctxt = terraformer::ui::wsapi::context::get();
	terraformer::ui::wsapi::native_window mainwin{
		gui_ctxt,
		"Terraformer",
		terraformer::ui::drawing_api::gl_surface_configuration{
			.api_version{
				.major = 4,
				.minor = 6
			}
		}
	};

	my_event_handler eh;
	mainwin.set_event_handler(std::ref(eh));
	gui_ctxt.wait_events(std::ref(eh), std::ref(mainwin));
}
