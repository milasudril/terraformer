//@	{
//@		"dependencies":[{"ref": "imgui", "origin":"pkg-config"}]
//@	}

#ifndef TERRAFORMER_UI_WIDGETS_CLIENT_WINDOW_HPP
#define TERRAFORMER_UI_WIDGETS_CLIENT_WINDOW_HPP

#include "./toolkit_instance.hpp"

namespace terraformer::ui::widgets
{
	struct window_geometry_configuration
	{
		float loc_x{0.0f};
		float loc_y{0.0f};
		float width{300};
		float height{500};
	};

	enum class window_features{};

	constexpr window_features operator~(window_features value)
	{ return static_cast<window_features>(~static_cast<uint32_t>(value)); }

	constexpr window_features operator|(window_features a, window_features b)
	{ return static_cast<window_features>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }

	constexpr window_features& operator|=(window_features& a, window_features b)
	{ return a = a | b; }

	constexpr window_features operator&(window_features a, window_features b)
	{ return static_cast<window_features>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b)); }

	constexpr window_features& operator&=(window_features& a, window_features b)
	{ return a = a & b; }

	constexpr window_features operator^(window_features a, window_features b)
	{ return static_cast<window_features>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b)); }

	constexpr auto is_set(window_features a, window_features feature)
	{ return static_cast<bool>(a & feature); }

	struct window_configuration
	{
		window_geometry_configuration geometry;
		window_features features{};
	};

	class client_window
	{
	public:
		[[nodiscard]] explicit client_window(
			toolkit_instance& tk,
			char const* title = nullptr,
			window_configuration const& cfg = window_configuration{}
		):
		m_id{tk.create_widget_id()},
		m_title{title == nullptr? "" : title},
		m_cfg{cfg}
		{
			ImGui::SetNextWindowPos(ImVec2{cfg.geometry.loc_x, cfg.geometry.loc_y});
			ImGui::SetNextWindowSize(
				ImVec2{
					cfg.geometry.width,
					cfg.geometry.height
				}
			);
			ImGui::Begin(toolkit_instance::make_name(title, m_id).c_str()
				,nullptr
				,ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar
				|ImGuiWindowFlags_::ImGuiWindowFlags_NoResize
				|ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar
				|ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar
			);
		}

		~client_window()
		{ ImGui::End(); }

	private:
		uint64_t m_id;
		std::string m_title;
		window_configuration m_cfg;
	};
}

#endif