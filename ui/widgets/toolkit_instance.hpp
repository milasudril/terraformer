//@	{
//@		"dependencies":[{"ref": "imgui", "origin":"pkg-config"}]
//@	}

#ifndef TERRAFORMER_UI_WIDGETS_TOOLKIT_INSTANCE_HPP
#define TERRAFORMER_UI_WIDGETS_TOOLKIT_INSTANCE_HPP

#include "ui/font_handling/font_mapper.hpp"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

namespace terraformer::ui::widgets
{
	class toolkit_instance
	{
	public:
		[[nodiscard]] explicit toolkit_instance(GLFWwindow* window)
		{
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			auto io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.Fonts->AddFontFromFileTTF(font_handling::font_mapper{}.get_path("sans-serif").c_str(), 16);
			ImGui_ImplOpenGL3_Init();
			ImGui_ImplGlfw_InitForOpenGL(window, true);
		}

		void prepare_frame()
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}

		void finalize_frame()
		{
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		~toolkit_instance()
		{
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}

	private:
	};
}

#endif