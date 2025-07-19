#ifndef TERRAFORMER_UI_WIDGETS_FLOAT_INPUT_HPP
#define TERRAFORMER_UI_WIDGETS_FLOAT_INPUT_HPP

#include "./widget_group.hpp"
#include "./text_to_float_input.hpp"
#include "ui/layouts/table.hpp"
#include "ui/main/widget_collection.hpp"

namespace terraformer::ui::widgets
{
	template<class ControlWidget>
	class float_input:private widget_group<layouts::table>
	{
	public:
		using widget_group::handle_event;
		using widget_group::prepare_for_presentation;
		using widget_group::theme_updated;
		using widget_group::get_children;
		using widget_group::get_layout;
		using widget_group::compute_size;
		using widget_group::confirm_size;

		explicit float_input(
			iterator_invalidation_handler_ref iihr,
			ControlWidget&& input_widget = ControlWidget{},
			main::widget_orientation orientation = main::widget_orientation::horizontal
		):
			widget_group{
				iihr,
				2u,
				orientation == main::widget_orientation::vertical?
					layouts::table::cell_order::column_major:
					layouts::table::cell_order::row_major
			},
			m_input_widget{std::move(input_widget)}
		{ init(); }

		template<class Function>
		float_input& on_value_changed(Function&& f)
		{
			m_on_value_changed = std::forward<Function>(f);
			return *this;
		}

		float value() const
		{ return m_input_widget.value(); }

		float_input& value(float new_val)
		{
			m_input_widget.value(new_val);
			m_textbox.value(new_val);
			return *this;
		}

		ControlWidget& input_widget()
		{ return m_input_widget; }

		ControlWidget const& input_widget() const
		{ return m_input_widget; }

		template<class StringType>
		float_input& set_textbox_placeholder_string(StringType&& placeholder)
		{
			m_textbox.use_size_from_placeholder(std::forward<StringType>(placeholder));
			return *this;
		}

		// TODO: It could be a good idea to only show ControlWidget when we have keyboard/mouse focus
		// This will save precious space

	private:
		ControlWidget m_input_widget;
		text_to_float_input m_textbox;
		main::widget_user_interaction_handler<float_input> m_on_value_changed{no_operation_tag{}};

		void init()
		{
			append(std::ref(m_input_widget), terraformer::ui::main::widget_geometry{});
			append(std::ref(m_textbox), terraformer::ui::main::widget_geometry{});
			m_input_widget.on_value_changed([this]<class ... Args>(auto const& input, Args&&... args){
				m_textbox.value(input.value());
				m_on_value_changed(*this, std::forward<Args>(args)...);
			});
			m_textbox
				.on_value_changed([this]<class ... Args>(auto& input, Args&&... args){
					m_input_widget.value(input.value());
					input.value(m_input_widget.value());
					m_on_value_changed(*this, std::forward<Args>(args)...);
				})
				.value(m_input_widget.value());
			layout.params().no_outer_margin = true;
			is_transparent = true;
			if constexpr (requires(){{ControlWidget::expanded_by_default} -> std::convertible_to<bool>;})
			{
				if constexpr (ControlWidget::expanded_by_default)
				{
					layout.set_cell_size(0,  terraformer::ui::layouts::table::cell_size::expand{}); 
					auto const attribs = get_attributes();
					auto const index = attribs.element_indices().front();
					attribs.widget_states()[index].maximized = true;
				}
			}
		}
	};
}

#endif
