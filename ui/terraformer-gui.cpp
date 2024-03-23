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
	bool do_main_iteration(terraformer::gl_viewport& viewport)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow(); // Show demo window! :)

		glClear(GL_COLOR_BUFFER_BIT);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		viewport.swap_buffers();
		return false;
	}
}

int main(int, char**)
{
	auto& gui_ctxt = terraformer::glfw_context::get();
	terraformer::gl_viewport mainwin{gui_ctxt, 800, 500, "Terraformer"};

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui_ImplGlfw_InitForOpenGL(mainwin.handle(), true);
	ImGui_ImplOpenGL3_Init();

	gui_ctxt.read_events(do_main_iteration, std::ref(mainwin));

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
