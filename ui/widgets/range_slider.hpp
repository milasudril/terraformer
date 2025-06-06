//@	{"dependencies_extra":[{"ref":"./range_slider.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_RANGE_SLIDER_HPP
#define TERRAFORMER_UI_WIDGETS_RANGE_SLIDER_HPP

#include "./value_map.hpp"
#include "ui/main/texture_types.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"
#include "ui/value_maps/affine_value_map.hpp"

#include "lib/pixel_store/rgba_pixel.hpp"
#include "lib/common/interval.hpp"

namespace terraformer::ui::widgets
{
	class range_slider:public main::widget_with_default_actions
	{
	public:
		using widget_with_default_actions::handle_event;

		range_slider() = default;

		template<class ValueMap>
		requires (!std::is_same_v<std::remove_cvref_t<ValueMap>, range_slider>)
		explicit range_slider(ValueMap&& vm):
			m_value_map{std::forward<ValueMap>(vm)}
		{ value(closed_closed_interval<float>{0.0f, 1.0f}); }

		template<class ValueMap, class... Args>
		explicit range_slider(std::in_place_type_t<ValueMap>, Args&&... args):
			m_value_map{std::in_place_type_t<ValueMap>{}, std::forward<Args>(args)...}
		{ value(closed_closed_interval<float>{0.0f, 1.0f}); }

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
		{
			auto const ptr = m_value_map.get().get_pointer();
			auto const mapping_function = m_value_map.get().get_vtable().to_value;
			return closed_closed_interval{
				mapping_function(ptr, m_current_range.min()),
				mapping_function(ptr, m_current_range.max())
			};
		}

		range_slider& value(closed_closed_interval<float> new_val)
		{
			auto const ptr = m_value_map.get().get_pointer();
			auto const mapping_function = m_value_map.get().get_vtable().from_value;
			auto const mapped_min = std::clamp(mapping_function(ptr, new_val.min()), 0.0f, 1.0f);
			auto const mapped_max = std::clamp(mapping_function(ptr, new_val.max()), 0.0f, 1.0f);

			m_current_range = closed_closed_interval{
				std::min(mapped_min, mapped_max),
				std::max(mapped_min, mapped_max)
			};
			m_dirty_bits |= selection_dirty;
			return *this;
		}

		template<class Function>
		range_slider& on_value_changed(Function&& f)
		{
			m_on_value_changed = std::forward<Function>(f);
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

		type_erased_value_map m_value_map{
			std::in_place_type_t<value_maps::affine_value_map>{}, 0.0f, 1.0f
		};

		main::widget_user_interaction_handler<range_slider> m_on_value_changed{no_operation_tag{}};
	};
}

#endif
