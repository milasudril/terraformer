#ifndef TERRAFORMER_UI_WIDGETS_INTERVAL_INPUT_HPP
#define TERRAFORMER_UI_WIDGETS_INTERVAL_INPUT_HPP

#include "./widget_group.hpp"
#include "./text_to_float_input.hpp"
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


		explicit interval_input(
			iterator_invalidation_handler_ref iihr,
			main::widget_orientation orientation = main::widget_orientation::horizontal
		):
			widget_group{
				iihr,
				2u,
				orientation == main::widget_orientation::vertical?
					layouts::table::cell_order::column_major:
					layouts::table::cell_order::row_major
			}
		{
			append(std::ref(m_textbox_lower), terraformer::ui::main::widget_geometry{});
			append(std::ref(m_textbox_upper), terraformer::ui::main::widget_geometry{});
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
			m_textbox_lower.on_value_changed(
				[this](
					single_line_text_input&,
					main::window_ref window,
					main::ui_controller ui_ctrl
				){
					m_on_value_changed(*this, window, ui_ctrl);
				}
			);
			m_textbox_upper.on_value_changed(
				[this](
					single_line_text_input&,
					main::window_ref window,
					main::ui_controller ui_ctrl
				){
					m_on_value_changed(*this, window, ui_ctrl);
				}
			);
			return *this;
		}


	private:
		text_to_float_input m_textbox_lower;
		text_to_float_input m_textbox_upper;

		main::widget_user_interaction_handler<interval_input> m_on_value_changed{no_operation_tag{}};

	};
}

#endif
