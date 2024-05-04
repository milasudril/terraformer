//@	{
//@		"target":{"name":"terraformer-gui.o"}
//@	}

#include "./drawing_api/gl_surface_configuration.hpp"
#include "./drawing_api/gl_texture.hpp"
#include "./drawing_api/frame_renderer.hpp"
#include "./drawing_api/frame_renderer.hpp"
#include "./main/event_dispatcher.hpp"
#include "./widgets/workspace.hpp"
#include "./wsapi/native_window.hpp"
#include "./widgets/testwidget.hpp"
#include "./theming/cursor_set.hpp"
#include "./theming/default_stock_textures_repo.hpp"

namespace
{
	struct window_controller
	{
		template<auto>
		void window_is_closing()
		{ should_exit = true; }

		bool main_loop_should_exit(auto&&...) const
		{ return should_exit; }

		bool should_exit{false};
	};

	struct error_handler
	{
		template<auto>
		void error_detected(terraformer::ui::wsapi::error_message const& msg) noexcept
		{
			fprintf(stderr, "%s\n", msg.description.c_str());
		}
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

//	terraformer::ui::widgets::testwidget foo;
//	terraformer::ui::widgets::testwidget bar;

	terraformer::ui::widgets::workspace<
		terraformer::ui::main::default_stock_textures_repo<terraformer::ui::drawing_api::gl_texture>,
		terraformer::ui::drawing_api::frame_renderer::input_rectangle
	> my_workspace;
#if 0
	my_workspace.append(
		std::ref(foo),
		terraformer::ui::main::widget_geometry{
			.where = terraformer::location{50.0f, -25.0f, 0.0f},
			.origin = terraformer::location{-1.0f, 1.0f, 0.0f},
			.size = terraformer::scaling{150.0f, 100.0f, 0.0f}
		}
	);

	my_workspace.append(
		std::ref(bar),
		terraformer::ui::main::widget_geometry{
			.where = terraformer::location{250.0f, -25.0f, 0.0f},
			.origin = terraformer::location{-1.0f, 1.0f, 0.0f},
			.size = terraformer::scaling{150.0f, 100.0f, 0.0f}
		}
	);
#endif

	terraformer::ui::drawing_api::frame_renderer renderer{};

	terraformer::ui::main::event_dispatcher event_dispatcher{
		std::ref(my_workspace),
		window_controller{},
		std::ref(renderer),
		error_handler{}
	};

	auto const& widget_look = terraformer::ui::theming::default_widget_look;
	auto const texture_repo =
		terraformer::ui::main::generate_default_stock_textures<terraformer::ui::drawing_api::gl_texture>();

	auto const default_cursor = terraformer::ui::theming::create_cursor(
		gui_ctxt,
		terraformer::ui::theming::current_cursor_set.main,
		widget_look.colors.cursor_color
	);

	mainwin.set_cursor(default_cursor);

	mainwin.set_event_handler<0>(std::ref(event_dispatcher));
	gui_ctxt.wait_events(
		std::ref(event_dispatcher),
		std::ref(mainwin),
		std::ref(texture_repo),
		std::ref(widget_look)
	);
}
