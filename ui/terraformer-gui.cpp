//@	{
//@		"target":{"name":"terraformer-gui.o"}
//@	}

#include "./drawing_api/gl_surface_configuration.hpp"
#include "./drawing_api/gl_texture.hpp"
#include "./drawing_api/single_quad_renderer.hpp"
#include "./main/widget_list.hpp"
#include "./main/default_stock_textures_repo.hpp"
#include "./layouts/workspace.hpp"
#include "./wsapi/native_window.hpp"
#include "./theming/color_scheme.hpp"
#include "./widgets/testwidget.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/common/rng.hpp"

#include <random>

namespace
{
	struct my_event_handler
	{
		my_event_handler()
		{
			m_workspace.append(
				std::ref(m_foo),
				terraformer::ui::main::widget_geometry{
					.where = terraformer::location{50.0f, -25.0f, 0.0f},
					.origin= terraformer::location{-1.0f, 1.0f, 0.0f},
					.size = terraformer::scaling{150.0f, 100.0f, 0.0f}
				}
			);
		}

		void error_detected(terraformer::ui::wsapi::error_message const& msg) noexcept
		{
			fprintf(stderr, "%s\n", msg.description.c_str());
		}

		void handle_mouse_button_event(terraformer::ui::wsapi::mouse_button_event const& event)
		{
			m_workspace.handle_event(event);
		}

		void window_is_closing()
		{ should_close = true; }

		void handle_cursor_enter_leave_event(terraformer::ui::wsapi::cursor_enter_leave_event const&)
		{
		}

		void framebuffer_size_changed(terraformer::ui::wsapi::fb_size size)
		{
			fb_size = size;
			glViewport(0, 0, size.width, size.height);
			terraformer::ui::drawing_api::single_quad_renderer::get_default_instance()
				.set_world_transform(terraformer::location{-1.0f, 1.0f, 0.0f}, size);
		}

		bool operator()(
			terraformer::ui::wsapi::native_window<terraformer::ui::drawing_api::gl_surface_configuration>& viewport
		)
		{
			m_workspace.render();

			glClear(GL_COLOR_BUFFER_BIT);
			auto& renderer = terraformer::ui::drawing_api::single_quad_renderer::get_default_instance();
			renderer.render(
				terraformer::location{0.0f, 0.0f, 0.0f},
				terraformer::location{-1.0f, 1.0f, 0.0f},
				terraformer::scaling{static_cast<float>(fb_size.width), static_cast<float>(fb_size.height), 1.0f},
				m_workspace.background(),
				m_workspace.foreground()
			);

			m_workspace.show_widgets(renderer);

			viewport.swap_buffers();
			return should_close;
		}

		bool should_close{false};
		terraformer::ui::wsapi::fb_size fb_size;

		terraformer::ui::widgets::testwidget m_foo;

		terraformer::ui::layouts::workspace<
			terraformer::ui::main::default_stock_textures_repo<terraformer::ui::drawing_api::gl_texture>
		> m_workspace;
	};
}

int main(int, char**)
{
	auto& gui_ctxt = terraformer::ui::wsapi::context::get_instance();
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
