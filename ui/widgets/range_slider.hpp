//@	{"dependencies_extra":[{"ref":"./range_slider.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_RANGE_SLIDER_HPP
#define TERRAFORMER_UI_WIDGETS_RANGE_SLIDER_HPP

#include "lib/pixel_store/rgba_pixel.hpp"
#include "lib/common/interval.hpp"
#include "ui/main/texture_types.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"

namespace terraformer::ui::widgets
{
	class range_slider:public main::widget_with_default_actions
	{
	public:
		using widget_with_default_actions::handle_event;

		range_slider& orientation(main::widget_orientation new_orientation)
		{
			m_orientation = new_orientation;
			m_dirty_bits |= track_dirty | selection_dirty;
			return *this;
		}

		void regenerate_selection_mask();

		void regenerate_track();

		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend);

		box_size compute_size(main::widget_width_request req);

		box_size compute_size(main::widget_height_request req);

		box_size confirm_size(box_size size)
		{
			m_current_size = main::fb_size{
				.width = static_cast<int>(size[0]),
				.height = static_cast<int>(size[1])
			};
			m_dirty_bits |= track_dirty | selection_dirty;
			return size;
		}

		void theme_updated(main::config const& cfg, main::widget_instance_info);

		closed_closed_interval<float> value() const
		{ return m_current_range; }

		range_slider& value(closed_closed_interval<float> new_val)
		{
			m_current_range = new_val;
			m_dirty_bits |= selection_dirty;
			return *this;
		}

	private:
		static constexpr unsigned int track_dirty = 0x1;
		static constexpr unsigned int selection_dirty = 0x2;

		unsigned int m_dirty_bits = track_dirty | selection_dirty;
		main::widget_orientation m_orientation = main::widget_orientation::horizontal;
		main::fb_size m_current_size;

		main::immutable_shared_texture m_null_texture;
		main::immutable_shared_texture m_background;
		main::unique_texture m_selection_mask;
		main::unique_texture m_frame;
		rgba_pixel m_bg_tint;
		rgba_pixel m_sel_tint;
		rgba_pixel m_fg_tint;
		float m_border_thickness;
		unsigned int m_track_size;
		closed_closed_interval<float> m_current_range{0.0f, 1.0f};
	};
}

#endif
