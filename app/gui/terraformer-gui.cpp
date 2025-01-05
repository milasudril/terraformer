//@	{
//@		"target":{"name":"terraformer-gui.o"}
//@	}

#include "ui/drawing_api/gl_surface_configuration.hpp"
#include "ui/drawing_api/gl_resource_factory.hpp"
#include "ui/drawing_api/gl_frame_renderer.hpp"
#include "ui/drawing_api/gl_widget_stack_renderer.hpp"
#include "ui/event_dispatcher/event_dispatcher.hpp"
#include "ui/theming/cursor_set.hpp"
#include "ui/theming/theme_loader.hpp"
#include "ui/wsapi/native_window.hpp"
#include "ui/widgets/float_input.hpp"
#include "ui/value_maps/log_value_map.hpp"
#include "ui/value_maps/asinh_value_map.hpp"
#include "ui/value_maps/qurt_value_map.hpp"
#include "ui/widgets/knob.hpp"
#include "ui/widgets/form.hpp"

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

namespace
{
	struct plain_corner
	{
		float elevation = 840.0f;
		float ddx = 0.0f;
		float ddy = 0.0f;
	};

	struct plain_corner_field_descriptor
	{
		std::u8string_view label;
		using input_widget_type = terraformer::ui::widgets::form;
	};

	void bind(plain_corner& point, terraformer::ui::widgets::form& form)
	{
		struct float_field_descriptor
		{
			std::u8string_view label;
			std::reference_wrapper<float> value_reference;
			using input_widget_type = terraformer::ui::widgets::float_input<
				terraformer::ui::widgets::knob
			>;
		};

		form.create_widget(
			float_field_descriptor{
				.label = u8"Elevation",
				.value_reference = std::ref(point.elevation)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::asinh_value_map{
					266.3185546307779f,
					0.7086205026374324f*6.0f
				}
			}
		);

		form.create_widget(
			float_field_descriptor{
				.label = u8"d/dx",
				.value_reference = std::ref(point.ddx)
			},
			terraformer::ui::widgets::knob{terraformer::ui::value_maps::qurt_value_map{3.0f}}
		);

		form.create_widget(
			float_field_descriptor{
				.label = u8"d/dy",
				.value_reference = std::ref(point.ddy)
			},
			terraformer::ui::widgets::knob{terraformer::ui::value_maps::qurt_value_map{3.0f}}
		);
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

	plain_corner nw;
	plain_corner ne;

	terraformer::ui::widgets::form main_form{
		terraformer::ui::main::widget_orientation::horizontal
	};

	{
		auto& subform = main_form.create_widget(
			plain_corner_field_descriptor{
				.label = u8"NW"
			},
			terraformer::ui::main::widget_orientation::vertical
		);
		bind(nw, subform);
	}

	{
		auto& subform = main_form.create_widget(
			plain_corner_field_descriptor{
				.label = u8"NE"
			},
			terraformer::ui::main::widget_orientation::horizontal
		);
		bind(ne, subform);
	}

	main_form.on_content_updated([&nw](auto&&...){
		printf("Content updated: %.8g\n", nw.elevation);
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
