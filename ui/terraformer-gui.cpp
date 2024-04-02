//@	{
//@		"target":{"name":"terraformer-gui.o"}
//@	}

#include "./drawing_api/gl_surface_configuration.hpp"
#include "./drawing_api/gl_texture.hpp"
#include "./drawing_api/single_quad_renderer.hpp"
#include "./main/widget_list.hpp"
#include "./layouts/workspace.hpp"
#include "./wsapi/native_window.hpp"

namespace
{
	struct my_event_handler
	{
		terraformer::ui::drawing_api::single_quad_renderer m_quad;

		void window_is_closing()
		{ should_close = true; }

		void framebuffer_size_changed(terraformer::ui::wsapi::fb_size size)
		{
			fb_size = size;
			glViewport(0, 0, size.width, size.height);
			m_quad.set_world_transform(terraformer::location{-1.0f, 1.0f, 0.0f}, size);
		}

		bool operator()(
			terraformer::ui::wsapi::native_window<terraformer::ui::drawing_api::gl_surface_configuration>& viewport
		)
		{
			glClear(GL_COLOR_BUFFER_BIT);
			auto const loc = viewport.get_cursor_position();

			m_quad.render(
				terraformer::location{static_cast<float>(loc.x), -static_cast<float>(loc.y), 0.0f},
				terraformer::location{0.0f, 0.0f, 0.0f},
				terraformer::scaling{200.0f, 125.0f, 1.0f}
			);
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

	glEnable(GL_CULL_FACE);
	my_event_handler eh;
	mainwin.set_event_handler(std::ref(eh));
	gui_ctxt.wait_events(std::ref(eh), std::ref(mainwin));
}
