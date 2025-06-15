//@	{"dependencies_extra":[{"ref":"./label.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_LABEL_HPP
#define TERRAFORMER_UI_WIDGETS_LABEL_HPP

#include "ui/font_handling/text_shaper.hpp"
#include "ui/main/widget.hpp"
#include "lib/common/object_tree.hpp"

namespace terraformer::ui::widgets
{
	class label:public main::widget_with_default_actions
	{
	public:
		using widget_with_default_actions::handle_event;

		template<class StringType>
		label& text(StringType&& str)
		{
			m_text = std::forward<StringType>(str);
			m_dirty_bits |= text_dirty | host_textures_dirty;
			return *this;
		}

		template<class StringType>
		label& value(StringType&& str)
		{ return text(std::forward<StringType>(str)); }

		std::basic_string_view<char8_t> value() const
		{ return m_text; }

		void regenerate_text_mask();

		void regenerate_textures();

		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend);

		box_size compute_size(main::widget_width_request req);

		box_size compute_size(main::widget_height_request req);

		box_size confirm_size(box_size size)
		{
			m_current_size = main::fb_size{
				.width = static_cast<int>(size[0]),
				.height = static_cast<int>(size[1])
			};
			m_dirty_bits |= host_textures_dirty;

			return size;
		}

		void theme_updated(const main::config& cfg, main::widget_instance_info);

		void set_margin(float new_val)
		{ m_margin = new_val; }

		uint32_t text_height() const
		{ return m_rendered_text.height(); }

		uint32_t text_width() const
		{ return m_rendered_text.width(); }

	private:
		std::basic_string<char8_t> m_text;
		basic_image<uint8_t> m_rendered_text;

		// TODO: Cleanup flags
		static constexpr auto text_dirty = 0x1;
		static constexpr auto host_textures_dirty = 0x2;
		unsigned int m_dirty_bits = text_dirty | host_textures_dirty;

		float m_margin = 0;
		std::shared_ptr<font_handling::font const> m_font;
		rgba_pixel m_fg_tint;

		main::unique_texture m_foreground;
		main::immutable_shared_texture m_null_texture;

		main::fb_size m_current_size;
	};

	class interactive_label:private label
	{
	public:
		enum class state{released, pressed};

		using label::label;
		using label::handle_event;
		using label::prepare_for_presentation;
		using label::text;
		using label::theme_updated;
		using label::get_layout;
		using label::get_children;
		using label::compute_size;
		using label::confirm_size;
		using label::value;

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

		template<class Function>
		interactive_label& on_activated(Function&& func)
		{
			m_on_activated = std::forward<Function>(func);
			return *this;
		}

	private:
		main::widget_user_interaction_handler<interactive_label> m_on_activated{no_operation_tag{}};
		state m_state_to_display = state::released;
		state m_state_saved = state::released;
	};
}

#endif
