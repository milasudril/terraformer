//@	{
//@		"target":{"name":"terraformer-gui.o"}
//@	}

#include "./renderer/gl_surface_configuration.hpp"
#include "./wsapi/native_window.hpp"
#include "./widgets/client_window.hpp"

namespace
{
	struct my_event_handler
	{
		void window_is_closing()
		{ should_close = true; }

		void framebuffer_size_changed(terraformer::ui::wsapi::fb_size size)
		{fb_size = size;}

		bool operator()(
			terraformer::ui::wsapi::native_window<terraformer::ui::renderer::gl_surface_configuration>& viewport,
			terraformer::ui::widgets::toolkit_instance& tk
		)
		{
			tk.prepare_frame();
			{
				terraformer::ui::widgets::client_window window{
					tk,
					nullptr,
					terraformer::ui::widgets::window_configuration{
						.geometry{
							.loc_x = 0.0f,
							.loc_y = 0.0f,
							.width = static_cast<float>(fb_size.width),
							.height = static_cast<float>(fb_size.height)
						}
					}
				};

				for(size_t k = 0; k != 10; ++k)
				{
					ImGui::Button(std::to_string(k).append(" A Button sdajkflase foeisaj pseaifj espaof jesapf ").c_str());
				}
			}

			glClear(GL_COLOR_BUFFER_BIT);
			tk.finalize_frame();

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
		terraformer::ui::renderer::gl_surface_configuration{
			.api_version{
				.major = 4,
				.minor = 6
			}
		}
	};

	terraformer::ui::widgets::toolkit_instance toolkit{mainwin.handle()};

	my_event_handler eh;
	mainwin.set_event_handler(std::ref(eh));
	gui_ctxt.wait_events(std::ref(eh), std::ref(mainwin), std::ref(toolkit));
}
