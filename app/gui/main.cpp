//@	{
//@		"target":{"name": "main.o"}
//@	}

#include "./plain.hpp"
#include "./elevation_color_map.hpp"

#include "ui/drawing_api/gl_surface_configuration.hpp"
#include "ui/drawing_api/gl_resource_factory.hpp"
#include "ui/drawing_api/gl_frame_renderer.hpp"
#include "ui/drawing_api/gl_widget_stack_renderer.hpp"
#include "ui/event_dispatcher/event_dispatcher.hpp"
#include "ui/theming/cursor_set.hpp"
#include "ui/theming/theme_loader.hpp"
#include "ui/wsapi/native_window.hpp"
#include "ui/widgets/form.hpp"
#include "ui/widgets/false_color_image_view.hpp"
#include "ui/widgets/colorbar.hpp"

#include "lib/pixel_store/image_io.hpp"
#include "lib/execution/task_receiver.hpp"
#include "lib/execution/notifying_task.hpp"

namespace
{
	struct mainwin_tag
	{};

	struct window_controller
	{
		void handle_event(mainwin_tag, terraformer::ui::main::window_close_event)
		{ should_exit = true; }

		template<class ... T>
		void handle_event(T&&...)
		{}

		bool main_loop_should_exit(auto&&...) const
		{ return should_exit; }

		bool should_exit{false};
	};

	struct error_handler
	{
		void handle_event(mainwin_tag, terraformer::ui::main::error_message const& msg) noexcept
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
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( MessageCallback, 0 );

	terraformer::plain_descriptor plain;

	terraformer::ui::widgets::form main_form{
		terraformer::ui::main::widget_orientation::horizontal
	};

	auto& plain_form = terraformer::app::bind(u8"Plain settings", plain, main_form);

	terraformer::task_receiver<terraformer::move_only_function<void()>> task_receiver;

	struct heightmap_field_descriptor
	{
		std::u8string_view label;
		using input_widget_type = terraformer::ui::widgets::false_color_image_view;
	};
	auto& heightmap_view = main_form.create_widget(
		heightmap_field_descriptor{
			.label = u8"Current heightmap"
		},
		terraformer::global_elevation_map,
		terraformer::get_elevation_color_lut()
	);

	struct colorbar_field_descriptor
	{
		std::u8string_view label;
		using input_widget_type = terraformer::ui::widgets::colorbar;
	};
	main_form.create_widget(
		colorbar_field_descriptor{
			.label = u8""
		},
		terraformer::global_elevation_map,
		terraformer::get_elevation_color_lut()
	);

	plain_form.on_content_updated([&plain, &task_receiver, &heightmap_view, &gui_ctxt]<class ... Args>(Args&&...){
		printf("Event: %u\n", gettid());
		task_receiver.replace_pending_task(
			[&plain, &heightmap_view, &gui_ctxt]() {
				terraformer::grayscale_image output{512, 512};
				replace_pixels(output.pixels(), 96.0f, plain);
				heightmap_view.show_image(output);
				gui_ctxt.notify_main_loop();
				//store(output, "/dev/shm/slask.exr");
			}
		);
	});

	main_form.on_content_updated([](auto&&...){
		printf("Main: Content updated\n");
	});

	terraformer::ui::drawing_api::gl_resource_factory res_factory{};

	terraformer::ui::main::event_dispatcher event_dispatcher{
		terraformer::ui::theming::load_default_config(),
		window_controller{},
		terraformer::ui::drawing_api::gl_widget_layer_stack_renderer{},
		terraformer::ui::drawing_api::gl_frame_renderer{},
		error_handler{},
		std::ref(main_form)
	};

	mainwin.set_event_handler<mainwin_tag>(std::ref(event_dispatcher));
	gui_ctxt.wait_events(
		std::ref(event_dispatcher),
		std::ref(mainwin),
		terraformer::ui::main::graphics_backend_ref{res_factory}
	);
}
