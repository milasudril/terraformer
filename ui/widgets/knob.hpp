//@	{"dependencies_extra":[{"ref":"./knob.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_KNOB_HPP
#define TERRAFORMER_UI_WIDGETS_KNOB_HPP

#include "./float_input_controller.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"

namespace terraformer::ui::widgets
{
	class knob:public float_input_controller<knob>
	{
	public:
		using float_input_controller<knob>::handle_event;

		using float_input_controller<knob>::float_input_controller;

		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend);

		scaling compute_size(main::widget_width_request req);

		scaling compute_size(main::widget_height_request req);

		void handle_event(main::fb_size size)
		{ m_current_size = size; }

		void theme_updated(main::config const& cfg, main::widget_instance_info);

		knob& visual_angle_range(closed_closed_interval<geosimd::turn_angle> new_range)
		{
			m_angle_range = new_range;
			return *this;
		}

		internal_value_type to_internal_value(main::cursor_position loc) const
		{
			auto const dx = loc.x - 0.5f*static_cast<double>(m_current_size.width);
			auto const dy = loc.y + 0.5f*static_cast<double>(m_current_size.height);
			auto const theta = geosimd::turn_angle{
				geosimd::rad{std::atan2(dx, dy) + std::numbers::pi_v<double>}
			} - m_angle_range.min();

			return internal_value_type{
				static_cast<float>(
					to_turns(theta).value/
					to_turns(m_angle_range.max() - m_angle_range.min()).value
				),
				clamp_tag{}
			};
		}

	private:
		closed_closed_interval<geosimd::turn_angle> m_angle_range{
			geosimd::turns{1.0/6.0},
			geosimd::turns{5.0/6.0}
		};
		rgba_pixel m_bg_tint{};
		rgba_pixel m_fg_tint{};

		main::immutable_shared_texture m_null_texture;
		main::immutable_shared_texture m_handle;
		main::immutable_shared_texture m_hand;

		main::fb_size m_current_size{};
	};
}

#endif
