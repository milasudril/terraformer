//@	{
//@		"target":{"name":"terraformer-gui.o"}
//@	}

#include "./drawing_api/gl_surface_configuration.hpp"
#include "./drawing_api/gl_texture.hpp"
#include "./drawing_api/frame_renderer.hpp"
#include "./drawing_api/single_quad_renderer.hpp"
#include "./main/event_dispatcher.hpp"
#include "./widgets/workspace.hpp"
#include "./wsapi/native_window.hpp"
#include "./widgets/label.hpp"
#include "./widgets/button.hpp"
#include "./widgets/vbox.hpp"
#include "./theming/cursor_set.hpp"
#include "./theming/theme_loader.hpp"

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

void MessageCallback( GLenum,
									GLenum type,
									GLuint,
									GLenum severity,
									GLsizei,
									const GLchar* message,
									const void*)
	{
		fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
						( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
							type, severity, message );
	}
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( MessageCallback, 0 );

	terraformer::ui::widgets::label foo;
	foo.text(u8"Hello, World");
	terraformer::ui::widgets::toggle_button bar;
	bar.text(u8"Click me!")
		.value(false)
		.on_value_changed([&foo](auto const& obj){
			if(obj.value())
			{ foo.value(u8"Value is true");}
			else
			{ foo.value(u8"Value is false");}
		});
	terraformer::ui::widgets::vbox<
		terraformer::ui::drawing_api::single_quad_renderer::input_rectangle
	> my_vbox;

	my_vbox.append(std::ref(foo));
	my_vbox.append(std::ref(bar));

	terraformer::ui::main::event_dispatcher event_dispatcher{
		terraformer::ui::theming::load_default_resources<terraformer::ui::drawing_api::gl_texture>(),
		std::ref(my_vbox),
		window_controller{},
		terraformer::ui::drawing_api::single_quad_renderer{},
		terraformer::ui::drawing_api::frame_renderer{},
		error_handler{}
	};

	mainwin.set_event_handler<0>(std::ref(event_dispatcher));
	gui_ctxt.wait_events(
		std::ref(event_dispatcher),
		std::ref(mainwin)
	);
}
