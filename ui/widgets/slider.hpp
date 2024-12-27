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
			if(new_orientation == orientation::vertical)
			{ m_handle = m_horizontal_handle; } 
			else
			{ m_handle = m_vertical_handle; }
			m_orientation = new_orientation;
			m_dirty_bits |= track_dirty;
			return *this;
		}

		void regenerate_text_mask();

		void regenerate_textures();

		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend);
		
		void handle_event(main::cursor_motion_event const& cme, main::window_ref, main::ui_controller) 
		{
			if(m_state_current == state::handle_grabbed)
			{ value(to_value(cme.where)); }
		}
		
		void value(float new_val)
		{ m_value = std::clamp(new_val, 0.0f, 1.0f); }
		
		float value() const
		{ return m_value; }

		void handle_event(main::mouse_button_event const& mbe, main::window_ref, main::ui_controller)
		{
			if(mbe.button == 0)
			{
				switch(mbe.action)
				{
					case main::mouse_button_action::press:
						value(to_value(mbe.where));
						m_state_current = state::handle_grabbed;
						break;

					case main::mouse_button_action::release:
						puts("Release");
						m_state_current = state::released;
						break;
				}
			}
		}

		void handle_event(main::keyboard_button_event const&, main::window_ref, main::ui_controller)
		{
			// TODO
		}

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
		static constexpr unsigned int track_dirty = 0x1;
		unsigned int m_dirty_bits = track_dirty;

		unsigned int m_border_thickness = 0;
		std::shared_ptr<font_handling::font const> m_font;
		
		float track_margin() const
		{ return static_cast<float>(m_handle->frontend_resource().width())/2.0f; }
		
		float track_length() const
		{ return static_cast<float>(m_current_size.width) - 2.0f*track_margin(); }
		
		float to_value(main::cursor_position loc) const
		{ return static_cast<float>(loc.x - track_margin())/track_length(); }

		rgba_pixel m_bg_tint;
		rgba_pixel m_fg_tint;

		main::immutable_shared_texture m_null_texture;
		main::unique_texture m_track;

		main::immutable_shared_texture m_handle;
		main::immutable_shared_texture m_horizontal_handle;
		main::immutable_shared_texture m_vertical_handle;

		main::fb_size m_current_size;

		state m_state_current = state::released;
		float m_value = 0.0f;
		enum orientation m_orientation = orientation::horizontal;
	};
}

#endif
