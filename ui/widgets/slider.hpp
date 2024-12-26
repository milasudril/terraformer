//@	{"dependencies_extra":[{"ref":"./slider.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_SLIDER_HPP
#define TERRAFORMER_UI_WIDGETS_SLIDER_HPP

#include "ui/font_handling/text_shaper.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"
#include "lib/common/move_only_function.hpp"

namespace terraformer::ui::widgets
{
	class slider:public main::widget_with_default_actions
	{
	public:
		using widget_with_default_actions::handle_event;

		enum class state{released, handle_grabbed};
		enum class orientation{horizontal, vertical};

		slider& orientation(enum orientation new_orientation)
		{
			m_orientation = new_orientation;
			m_dirty_bits |= handle_dirty|track_dirty;
			return *this;
		}

		slider& value(bool new_value)
		{
			m_value = new_value;
			m_state_current = new_value? state::handle_grabbed: state::released;
			return *this;
		}

		bool value() const
		{ return m_value; }

		void regenerate_text_mask();

		void regenerate_textures();

		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend);

		void handle_event(main::cursor_leave_event const&, main::window_ref, main::ui_controller)
		{ m_state_saved = std::exchange(m_state_current, state::released); }

		void handle_event(main::cursor_enter_event const&, main::window_ref, main::ui_controller)
		{ m_state_current = m_state_saved; }
		
		void handle_event(main::cursor_motion_event const& event, main::window_ref, main::ui_controller) 
		{
			if(m_state_current == state::handle_grabbed)
			{
				m_value = static_cast<float>(event.where.x)/static_cast<float>(m_track.frontend_resource().width());
			}
		}

		void handle_event(main::mouse_button_event const& mbe, main::window_ref, main::ui_controller)
		{
			if(mbe.button == 0)
			{
				switch(mbe.action)
				{
					case main::mouse_button_action::press:
						m_value = static_cast<float>(mbe.where.x)/static_cast<float>(m_track.frontend_resource().width());
						m_state_current = state::handle_grabbed;
						break;

					case main::mouse_button_action::release:
						m_state_current = state::released;
						m_state_saved = state::released;
						break;
				}
			}
		}

		void handle_event(main::keyboard_button_event const&, main::window_ref, main::ui_controller)
		{
			// TODO
		}

		void handle_event(main::keyboard_focus_leave_event, main::window_ref, main::ui_controller)
		{ m_state_saved = std::exchange(m_state_current, state::released); }

		scaling compute_size(main::widget_width_request req);

		scaling compute_size(main::widget_height_request req);

		void handle_event(main::fb_size size)
		{
			m_current_size = size;
			m_dirty_bits |= track_dirty;
		}

		void theme_updated(main::config const& cfg, main::widget_instance_info);

		main::layout_policy_ref get_layout() const
		{ return main::layout_policy_ref{}; }

		main::widget_collection_ref get_children()
		{ return main::widget_collection_ref{}; }

		main::widget_collection_view get_children() const
		{ return main::widget_collection_view{}; }

	private:
		main::widget_user_interaction_handler<slider> m_on_activated{no_operation_tag{}};
		static constexpr unsigned int handle_dirty = 0x1;
		static constexpr unsigned int track_dirty = 0x2;
		unsigned int m_dirty_bits = track_dirty;

		unsigned int m_border_thickness = 0;
		std::shared_ptr<font_handling::font const> m_font;

		float m_margin = 0.0f;
		rgba_pixel m_bg_tint;
		rgba_pixel m_fg_tint;

		main::immutable_shared_texture m_null_texture;
		main::unique_texture m_track;
		main::immutable_shared_texture m_handle;

		main::fb_size m_current_size;

		state m_state_saved = state::released;
		state m_state_current = state::released;
		float m_value = 0.0f;
		enum orientation m_orientation = orientation::horizontal;
	};
}

#endif
