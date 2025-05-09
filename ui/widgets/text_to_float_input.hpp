#ifndef TERRAFORMER_UI_WIDGETS_TEXT_TO_FLOAT_INPUT_HPP
#define TERRAFORMER_UI_WIDGETS_TEXT_TO_FLOAT_INPUT_HPP

#include "./single_line_text_input.hpp"

#include "lib/common/string_converter.hpp"

namespace terraformer::ui::widgets
{
	class text_to_float_input:private single_line_text_input
	{
	public:
		using single_line_text_input::handle_event;
		using single_line_text_input::prepare_for_presentation;
		using single_line_text_input::theme_updated;
		using single_line_text_input::get_layout;
		using single_line_text_input::get_children;
		using single_line_text_input::compute_size;
		using single_line_text_input::confirm_size;

		text_to_float_input()
		{ use_size_from_placeholder(u8"-1.175494351e-38"); }

		float value() const
		{
			auto tmp = single_line_text_input::value();
			for(auto& item : tmp)
			{
				if(item == ',')
				{ item = '.'; }
			}
			return num_string_converter<float>{}.convert(tmp);
		}

		text_to_float_input& value(float new_value)
		{
			single_line_text_input::value(num_string_converter<float>{}.convert(new_value));
			return *this;
		}

		template<class Function>
		text_to_float_input& on_value_changed(Function&& func)
		{
			single_line_text_input::on_edit_finalized(
				[cb = std::forward<Function>(func)](
					single_line_text_input& src,
					main::window_ref window,
					main::ui_controller ui_ctrl
				){
					cb(static_cast<text_to_float_input&>(src), window, ui_ctrl);
				}
			);
			return *this;
		}
	};
}

#endif