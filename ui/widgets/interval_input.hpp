#ifndef TERRAFORMER_UI_WIDGETS_INTERVAL_INPUT_HPP
#define TERRAFORMER_UI_WIDGETS_INTERVAL_INPUT_HPP

#include "./widget_group.hpp"
#include "./text_to_float_input.hpp"
#include "./range_slider.hpp"
#include "ui/layouts/table.hpp"
#include "ui/main/widget_collection.hpp"
#include "lib/common/interval.hpp"

namespace terraformer::ui::widgets
{
	class interval_input:private widget_group<layouts::table>
	{
	public:
		using widget_group::handle_event;
		using widget_group::prepare_for_presentation;
		using widget_group::theme_updated;
		using widget_group::get_children;
		using widget_group::get_layout;
		using widget_group::compute_size;
		using widget_group::confirm_size;

		template<class ValueMap>
		requires (!std::is_same_v<std::remove_cvref_t<ValueMap>, range_slider>)
		explicit interval_input(
			iterator_invalidation_handler_ref iihr,
			ValueMap&& vm
		):
			widget_group{
				iihr,
				1u,
				layouts::table::cell_order::row_major
			},
			m_slider{std::forward<ValueMap>(vm)},
			m_text_input{
				iihr,
				2u,
				layouts::table::cell_order::row_major
			}
		{
			append(std::ref(m_slider), main::widget_geometry{});
			m_text_input.append(std::ref(m_textbox_lower), main::widget_geometry{});
			m_text_input.append(std::ref(m_textbox_upper), ui::main::widget_geometry{});
			append(std::ref(m_text_input), main::widget_geometry{});
			layout.set_record_size(0, layouts::table::cell_size::expand{});
			auto const widget_attribs = get_attributes();
			widget_attribs.widget_states()[widget_attribs.element_indices().front()].maximized = true;
			m_text_input.layout.params().no_outer_margin = true;

			m_textbox_lower.on_value_changed(
				[this](
					single_line_text_input&,
					main::window_ref window,
					main::ui_controller ui_ctrl
				){
					m_slider.value(closed_closed_interval{m_textbox_lower.value(), m_textbox_upper.value()});
					m_on_value_changed(*this, window, ui_ctrl);
				}
			);
			m_textbox_upper.on_value_changed(
				[this](
					single_line_text_input&,
					main::window_ref window,
					main::ui_controller ui_ctrl
				){
					m_slider.value(closed_closed_interval{m_textbox_lower.value(), m_textbox_upper.value()});
					m_on_value_changed(*this, window, ui_ctrl);
				}
			);
		}

		closed_closed_interval<float> value() const
		{
			return closed_closed_interval{
				m_textbox_lower.value(),
				m_textbox_upper.value()
			};
		}

		interval_input& value(closed_closed_interval<float> new_val)
		{
			m_textbox_lower.value(new_val.min());
			m_textbox_upper.value(new_val.max());
			return *this;
		}

		template<class Function>
		interval_input& on_value_changed(Function&& func)
		{
			m_on_value_changed = std::forward<Function>(func);
			return *this;
		}


	private:
		range_slider m_slider;
		text_to_float_input m_textbox_lower;
		text_to_float_input m_textbox_upper;
		widget_group<layouts::table> m_text_input;

		main::widget_user_interaction_handler<interval_input> m_on_value_changed{no_operation_tag{}};

	};
}

#endif
