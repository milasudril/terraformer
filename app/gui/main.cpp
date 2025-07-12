//@	{
//@		"target":{"name": "main.o"}
//@	}

#include "./heightmap.hpp"
#include "./heightmap.hpp"
#include "./elevation_color_map.hpp"

#include "lib/descriptor_io/descriptor_editor_ref.hpp"
#include "ui/drawing_api/gl_surface_configuration.hpp"
#include "ui/drawing_api/gl_resource_factory.hpp"
#include "ui/drawing_api/gl_frame_renderer.hpp"
#include "ui/drawing_api/gl_widget_stack_renderer.hpp"
#include "ui/event_dispatcher/event_dispatcher.hpp"
#include "ui/main/widget_collection.hpp"
#include "ui/theming/cursor_set.hpp"
#include "ui/theming/theme_loader.hpp"
#include "ui/wsapi/native_window.hpp"
#include "ui/widgets/form.hpp"
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

	struct heightmap_form_field
	{
		std::u8string_view label;
		using input_widget_type = terraformer::ui::widgets::form;
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
		},
		terraformer::ui::wsapi::window_configuration{
			.geometry{
				.width = 1200,
				.height = 720
			}
		}
	};

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( MessageCallback, 0 );

	terraformer::ui::drawing_api::gl_resource_factory res_factory{};

	terraformer::ui::main::event_dispatcher event_dispatcher{
		terraformer::ui::theming::load_default_config(),
		window_controller{},
		terraformer::ui::drawing_api::gl_widget_layer_stack_renderer{},
		terraformer::ui::drawing_api::gl_frame_renderer{},
		error_handler{},
	};

	terraformer::ui::widgets::form main_form{
		terraformer::ui::main::widget_collection::iterator_invalidation_handler_ref{std::ref(event_dispatcher)},
		terraformer::ui::main::widget_orientation::horizontal
	};

	terraformer::heightmap_descriptor heightmap;
	auto& heightmap_form = main_form.create_widget(
		heightmap_form_field{
			.label = u8"Heightmap parameters"
		}
	);
	heightmap.bind(
		terraformer::descriptor_editor_ref{
			heightmap_form,
			std::type_identity<terraformer::app::descriptor_editor_traits>{}
		}
	);

	auto output = generate(heightmap);

	terraformer::app::heightmap_view_descriptor heightmap_view_info{
		.data = std::ref(output),
		.domain_size = std::ref(heightmap.domain_size),
		.heatmap_presentation_attributes{
			.level_curves = terraformer::app::level_curves_descriptor{
				.interval = 100.0f,
				.visible = true
			}
		},
		.xsection_presentation_attributes{}
	};

	auto& heightmap_view = main_form.create_widget(
		terraformer::app::heightmap_chart_form_field{
			.label = u8"Current heightmap",
			.expand_layout_cell = true
		}
	);

	terraformer::app::bind(heightmap_view_info, heightmap_view);

	terraformer::task_receiver<terraformer::move_only_function<void()>> task_receiver;

	heightmap_form.on_content_updated([&task_receiver, &heightmap_view, &gui_ctxt, &heightmap, &heightmap_img = output](auto&&...){
		task_receiver.replace_pending_task(
			[heightmap, &heightmap_img, &heightmap_view, &gui_ctxt]() {
				auto result = generate(heightmap);
				gui_ctxt
					.post_event([&heightmap_img, hm = result, &heightmap_view]() mutable {
						heightmap_img = std::move(hm);
						heightmap_view.refresh();
					})
					.notify_main_loop();
				store(result, "/dev/shm/slask.exr");
			}
		);
	});

	event_dispatcher.set_root_widget(std::ref(main_form));

	mainwin.set_event_handler<mainwin_tag>(std::ref(event_dispatcher));
	gui_ctxt.wait_events(
		std::ref(event_dispatcher),
		std::ref(mainwin),
		terraformer::ui::main::graphics_backend_ref{res_factory}
	);
}
