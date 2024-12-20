//@	{"dependencies_extra":[{"ref":"./button.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_BUTTON_HPP
#define TERRAFORMER_UI_WIDGETS_BUTTON_HPP

#include "ui/drawing_api/image_generators.hpp"
#include "ui/font_handling/text_shaper.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"
#include "lib/common/move_only_function.hpp"

namespace terraformer::ui::widgets
{
	class button:public main::widget_with_default_actions
	{
	public:
		using widget_with_default_actions::handle_event;

		enum class state{released, pressed};

		template<class Function>
		button& on_activated(Function&& func)
		{
			m_on_activated = std::forward<Function>(func);
			return *this;
		}

		template<class StringType>
		button& text(StringType&& text)
		{
			m_text = std::forward<StringType>(text);
			m_dirty_bits |= text_dirty;
			return *this;
		}

		button& value(bool new_value)
		{
			m_value = new_value;
			m_state_to_display = new_value? state::pressed : state::released;
			return *this;
		}

		bool value() const
		{ return m_value; }

		void regenerate_text_mask();

		void regenerate_textures();

		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend);

		void handle_event(main::cursor_leave_event const&, main::window_ref, main::ui_controller)
		{ m_state_saved = std::exchange(m_state_to_display, state::released); }

		void handle_event(main::cursor_enter_event const&, main::window_ref, main::ui_controller)
		{ m_state_to_display = m_state_saved; }

		void handle_event(main::mouse_button_event const& mbe, main::window_ref window, main::ui_controller ui_ctrl)
		{
			if(mbe.button == 0)
			{
				switch(mbe.action)
				{
					case main::mouse_button_action::press:
						m_state_to_display = state::pressed;
						break;

					case main::mouse_button_action::release:
						if(m_state_to_display == state::pressed)
						{ m_on_activated(*this, window, ui_ctrl); }
						m_state_to_display = state::released;
						m_state_saved = state::released;
						break;
				}
			}
		}

		void handle_event(main::keyboard_button_event const& kbe, main::window_ref window, main::ui_controller ui_ctrl)
		{
			switch(to_builtin_command_id(kbe))
			{
				case main::builtin_command_id::button_press:
					m_state_to_display = state::pressed;
					break;

				case main::builtin_command_id::button_release:
					if(m_state_to_display == state::pressed)
					{ m_on_activated(*this, window, ui_ctrl); }
					m_state_to_display = state::released;
					m_state_saved = state::released;
					break;

				default:
					break;
			}
		}

		void handle_event(main::keyboard_focus_leave_event, main::window_ref, main::ui_controller)
		{ m_state_saved = std::exchange(m_state_to_display, state::released); }

		scaling compute_size(main::widget_width_request req);

		scaling compute_size(main::widget_height_request req);

		void handle_event(main::fb_size size)
		{
			m_current_size = size;
			m_dirty_bits |= host_textures_dirty;
		}

		void theme_updated(main::config const& cfg, main::widget_instance_info);

		main::layout_policy_ref get_layout() const
		{ return main::layout_policy_ref{}; }

		main::widget_collection_ref get_children()
		{ return main::widget_collection_ref{}; }

		main::widget_collection_view get_children() const
		{ return main::widget_collection_view{}; }

	private:
		move_only_function<void(button&, main::window_ref, main::ui_controller)> m_on_activated =
			move_only_function<void(button&, main::window_ref, main::ui_controller)>{no_operation_tag{}};

		std::basic_string<char8_t> m_text;
		basic_image<uint8_t> m_rendered_text;

		// TODO: Cleanup flags
		static constexpr auto text_dirty = 0x1;
		static constexpr auto host_textures_dirty = 0x2;
		unsigned int m_dirty_bits = text_dirty | host_textures_dirty;

		float m_margin = 0;
		unsigned int m_border_thickness = 0;
		std::shared_ptr<font_handling::font const> m_font;
		rgba_pixel m_bg_tint;
		rgba_pixel m_fg_tint;

		main::unique_texture m_background_released;
		main::unique_texture m_background_pressed;
		main::unique_texture m_foreground;
		main::immutable_shared_texture m_null_texture;

		main::fb_size m_current_size;

		state m_state_to_display = state::released;
		state m_state_saved = state::released;
		bool m_value = false;
	};

	class toggle_button:private button
	{
	public:
		toggle_button()
		{
			button::on_activated(bound_callable<on_activated_callback>{});
		}

		using button::handle_event;
		using button::prepare_for_presentation;
		using button::text;
		using button::value;
		using button::theme_updated;
		using button::get_layout;
		using button::get_children;
		using button::compute_size;

		template<class Function>
		toggle_button& on_value_changed(Function&& func)
		{
			button::on_activated([cb = std::forward<Function>(func)](button& src, main::window_ref window, main::ui_controller ui_ctrl){
				on_activated_callback(src, window, ui_ctrl);
				cb(static_cast<toggle_button&>(src), window, ui_ctrl);
			});
			return *this;
		}

		template<class StringType>
		toggle_button& text(StringType&& text)
		{
			button::text(std::forward<StringType>(text));
			return *this;
		}

		toggle_button& value(bool new_value)
		{
			button::value(new_value);
			return *this;
		}

	private:
		static void on_activated_callback(button& source, main::window_ref, main::ui_controller)
		{ source.value(!source.value()); }
	};
}

#endif
