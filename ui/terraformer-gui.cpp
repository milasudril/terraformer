//@	{
//@		"target":{"name":"terraformer-gui.o"}
//@	}

#include "./drawing_api/gl_surface_configuration.hpp"
#include "./drawing_api/gl_resource_factory.hpp"
#include "./drawing_api/gl_frame_renderer.hpp"
#include "./drawing_api/gl_widget_stack_renderer.hpp"
#include "./event_dispatcher/event_dispatcher.hpp"
#include "./wsapi/native_window.hpp"
#include "./widgets/label.hpp"
#include "./widgets/button.hpp"
#include "./widgets/single_line_text_input.hpp"
#include "./widgets/slider.hpp"
#include "./widgets/vbox.hpp"
#include "./theming/cursor_set.hpp"
#include "./theming/theme_loader.hpp"
#include "./value_maps/log_value_map.hpp"
#include "./value_maps/asinh_value_map.hpp"

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

	terraformer::ui::widgets::label foo;
	foo.text(u8"Hello, World");
	terraformer::ui::widgets::toggle_button bar;
	bar.text(u8"Click me!")
		.value(false)
		.on_value_changed([&foo](auto const& obj, terraformer::ui::main::window_ref window, auto){
			if(obj.value())
			{
				foo.value(u8"Value is true");
				window.set_title(u8"True");
			}
			else
			{
				foo.value(u8"Value is false");
				window.set_title(u8"False");
			}
		});
	terraformer::ui::widgets::vbox my_vbox;

	my_vbox.append(std::ref(foo));
	my_vbox.append(std::ref(bar));

	terraformer::ui::widgets::vbox my_outer_vbox;

	terraformer::ui::widgets::button ok;
	ok.text(u8"Ok");

	terraformer::ui::widgets::button cancel;
	cancel.text(u8"Cancel");

	terraformer::ui::widgets::single_line_text_input my_input;
	my_input
		.use_size_from_placeholder(u8"Write your text here")
		.on_value_changed([](auto const& obj, terraformer::ui::main::window_ref window, auto){
				window.set_title(obj.value().c_str());
			}
		)
		.on_step_up([](auto&&...){
			puts("Step up");
		})
		.on_step_down([](auto&&...){
			puts("Step down");
		});

	terraformer::ui::widgets::label slider_label_asinh;
	slider_label_asinh.text(u8"A slider (asinh)");
	terraformer::ui::widgets::slider my_asinh_slider{
		std::in_place_type_t<terraformer::ui::value_maps::asinh_value_map>{},
		800.0f,
		10000.0f
	};
	my_asinh_slider.on_value_changed([](auto const& obj, auto&&...) {
		printf("%.8g\n", static_cast<float>(obj.value()));
	});

	terraformer::ui::widgets::label slider_label_log;
	slider_label_log.text(u8"A slider (log)");
	terraformer::ui::widgets::slider my_log_slider{
		std::in_place_type_t<terraformer::ui::value_maps::log_value_map>{},
		1.0f,
		10000.0f,
		10.0f
	};
	printf("Current value = %.8g\n", my_log_slider.value());

	my_log_slider.on_value_changed([](auto const& obj, auto&&...) {
		printf("%.8g\n", static_cast<float>(obj.value()));
	});
//	my_slider.orientation(terraformer::ui::widgets::slider::orientation::vertical);

	my_outer_vbox.append(std::ref(ok));
	my_outer_vbox.append(std::ref(my_vbox));
	my_outer_vbox.append(std::ref(cancel));
	my_outer_vbox.append(std::ref(my_input));
	my_outer_vbox.append(std::ref(slider_label_asinh));
	my_outer_vbox.append(std::ref(my_asinh_slider));
	my_outer_vbox.append(std::ref(slider_label_log));
	my_outer_vbox.append(std::ref(my_log_slider));


	terraformer::ui::drawing_api::gl_resource_factory res_factory{};

	terraformer::ui::main::event_dispatcher event_dispatcher{
		terraformer::ui::theming::load_default_config(),
		window_controller{},
		terraformer::ui::drawing_api::gl_widget_layer_stack_renderer{},
		terraformer::ui::drawing_api::gl_frame_renderer{},
		error_handler{},
		std::ref(my_outer_vbox)
	};

	mainwin.set_event_handler<mainwin_tag>(std::ref(event_dispatcher));
	gui_ctxt.wait_events(
		std::ref(event_dispatcher),
		std::ref(mainwin),
		terraformer::ui::main::graphics_backend_ref{res_factory}
	);
}
