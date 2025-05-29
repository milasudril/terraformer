#ifndef TERRAFORMER_UI_WIDGETS_TEXT_TO_BYTEARRAY_INPUT_HPP
#define TERRAFORMER_UI_WIDGETS_TEXT_TO_BYTEARRAY_INPUT_HPP

#include "./single_line_text_input.hpp"

#include "lib/common/string_converter.hpp"

namespace terraformer::ui::widgets
{
	template<size_t NumBytes>
	class text_to_bytearray_input:private single_line_text_input
	{
	public:
		using single_line_text_input::handle_event;
		using single_line_text_input::prepare_for_presentation;
		using single_line_text_input::theme_updated;
		using single_line_text_input::get_layout;
		using single_line_text_input::get_children;
		using single_line_text_input::compute_size;
		using single_line_text_input::confirm_size;

		explicit text_to_bytearray_input()
		{
			std::u8string placeholder;
			constexpr char8_t const* digits = u8"0A23456789ABCDEF";
			for(size_t k = 0; k != 2*NumBytes; ++k)
			{ placeholder += digits[k%16]; }
			use_size_from_placeholder(placeholder.c_str());
		}
		std::array<std::byte, NumBytes> value() const
		{
			auto tmp = single_line_text_input::value();
			if(std::size(tmp) != 2*NumBytes)
			{ throw std::runtime_error{"Invalid input"}; }

			std::array<std::byte, NumBytes> ret;
			auto ptr = reinterpret_cast<char const*>(std::data(tmp));
			auto res = hex_to_bytes(std::data(ret), ptr, NumBytes);
			if(res.ptr != ptr + 2*NumBytes)
			{ throw std::runtime_error{"Invalid input"}; }

			return ret;
		}

		text_to_bytearray_input& value(std::array<std::byte, NumBytes> const& new_val)
		{
			single_line_text_input::value(bytes_to_hex(reinterpret_cast<char const*>(std::data(new_val)), NumBytes));
			return *this;
		}

		template<class Function>
		text_to_bytearray_input& on_value_changed(Function&& func)
		{
			single_line_text_input::on_edit_finalized(
				[cb = std::forward<Function>(func)](
					single_line_text_input& src,
					main::window_ref window,
					main::ui_controller ui_ctrl
				){
					cb(static_cast<text_to_bytearray_input&>(src), window, ui_ctrl);
				}
			);
			return *this;
		}
	};
}

#endif