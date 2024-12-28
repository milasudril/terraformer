//@	{"dependencies_extra":[{"ref":"./slider.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_SLIDER_HPP
#define TERRAFORMER_UI_WIDGETS_SLIDER_HPP

#include "./float_input_controller.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"

namespace terraformer::ui::widgets
{
	class slider:public float_input_controller<slider>
	{
	public:
		using float_input_controller<slider>::handle_event;

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

		scaling compute_size(main::widget_width_request req);

		scaling compute_size(main::widget_height_request req);

		void handle_event(main::fb_size size)
		{
			m_current_size = size;
			m_dirty_bits |= track_dirty;
		}

		void theme_updated(main::config const& cfg, main::widget_instance_info);

		float to_value(main::cursor_position loc) const
		{
			if(m_orientation == orientation::horizontal)
			{ return static_cast<float>(loc.x - track_margin())/track_length(); }
			return static_cast<float>(loc.y + m_current_size.height - track_margin())/track_length();
		}

	private:
		static constexpr unsigned int track_dirty = 0x1;

		unsigned int m_dirty_bits = track_dirty;

		unsigned int m_border_thickness = 0;
		std::shared_ptr<font_handling::font const> m_font;

		float track_margin() const
		{
			if(m_orientation == orientation::horizontal)
			{ return static_cast<float>(m_handle->frontend_resource().width())/2.0f; }
			return static_cast<float>(m_handle->frontend_resource().height())/2.0f;
		}

		float track_length() const
		{
			if(m_orientation == orientation::horizontal)
			{ return static_cast<float>(m_current_size.width) - 2.0f*track_margin(); }
			return static_cast<float>(m_current_size.height) - 2.0f*track_margin();
		}

		rgba_pixel m_bg_tint;
		rgba_pixel m_fg_tint;

		main::immutable_shared_texture m_null_texture;
		main::unique_texture m_track;

		main::immutable_shared_texture m_handle;
		main::immutable_shared_texture m_horizontal_handle;
		main::immutable_shared_texture m_vertical_handle;

		main::fb_size m_current_size;

		state m_state_current = state::released;
		enum orientation m_orientation = orientation::horizontal;
	};
}

#endif
