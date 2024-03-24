//@	{
//@		"target":{"name":"terraformer-gui.o"},
//@		"dependencies":[{"ref": "imgui", "origin":"pkg-config"}]
//@	}

#include "./wsapi/gl_viewport.hpp"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

namespace
{
	struct my_event_handler
	{
		void window_is_closing()
		{ should_close = true; }

		void framebuffer_size_changed(terraformer::ui::wsapi::native_window::fb_size size)
		{fb_size = size;}

		bool operator()(terraformer::ui::wsapi::native_window& viewport)
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ImGui::SetNextWindowPos(ImVec2{0.0f, 0.0f});
			ImGui::SetNextWindowSize(
				ImVec2{
					static_cast<float>(fb_size.width),
					static_cast<float>(fb_size.height)
				}
			);
			ImGui::Begin("##mainwin"
				,nullptr
				,ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar
				|ImGuiWindowFlags_::ImGuiWindowFlags_NoResize
				|ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar
				|ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar
			);

			for(size_t k = 0; k != 10; ++k)
			{
				ImGui::Button(std::to_string(k).append(" A Button sdajkflase foeisaj pseaifj espaof jesapf ").c_str());
			}

			ImGui::End();
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			viewport.swap_buffers();
			return should_close;
		}

		bool should_close{false};
		terraformer::ui::wsapi::native_window::fb_size fb_size;
	};
}

int main(int, char**)
{
	auto& gui_ctxt = terraformer::ui::wsapi::context::get();
	terraformer::ui::wsapi::native_window mainwin{gui_ctxt, 800, 500, "Terraformer"};

	my_event_handler eh;
	mainwin.set_event_handler(std::ref(eh));

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui_ImplGlfw_InitForOpenGL(mainwin.handle(), true);
	ImGui_ImplOpenGL3_Init();

	gui_ctxt.read_events(std::ref(eh), std::ref(mainwin));

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
