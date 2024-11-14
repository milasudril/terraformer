//@	{"dependencies_extra":[{"ref":"./single_line_text_input.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_SINGLE_LINE_TEXT_INPUT_HPP
#define TERRAFORMER_UI_WIDGETS_SINGLE_LINE_TEXT_INPUT_HPP

#include "ui/main/generic_texture.hpp"
#include "ui/drawing_api/image_generators.hpp"
#include "ui/font_handling/text_shaper.hpp"
#include "ui/main/widget.hpp"
#include "lib/common/object_tree.hpp"
#include "lib/common/move_only_function.hpp"

#include <type_traits>

namespace terraformer::ui::widgets
{
	class single_line_text_input:public main::widget_with_default_actions
	{
	public:
		using widget_with_default_actions::handle_event;

		template<class Function>
		single_line_text_input& on_value_changed(Function&& func)
		{
			m_on_value_changed = std::forward<Function>(func);
			return *this;
		}

		template<class StringType>
		single_line_text_input& value(StringType&& new_val)
		{
			if constexpr(std::is_convertible_v<StringType, std::u32string>)
			{ m_value = std::forward<StringType>(new_val); }
			else
			{ m_value = to_utf32(new_val); }

			m_dirty_bits |= text_dirty;
			return *this;
		}

		auto value() const
		{ return to_utf8(m_value); }

		template<class StringType>
		single_line_text_input& use_size_from_placeholder(StringType&& placeholder)
		{
			m_placeholder = placeholder;
			return *this;
		}

		void handle_event(main::typing_event event, main::window_ref, main::ui_controller)
		{
			auto const i = std::begin(m_value) + m_insert_offset;
			update_insert_offset(m_value.insert(i, event.codepoint) + 1);
			m_dirty_bits |= text_dirty;
		}

		void handle_event(main::keyboard_button_event const& event, main::window_ref, main::ui_controller);

		void regenerate_text_mask();

		void regenerate_textures();

		void prepare_for_presentation(main::widget_rendering_result output_rect);

		scaling compute_size(main::widget_width_request req);

		scaling compute_size(main::widget_height_request req);

		void handle_event(main::fb_size size)
		{
			m_current_size = size;
			m_dirty_bits |= host_textures_dirty;
		}

		void theme_updated(main::config const& cfg, main::widget_instance_info);

	private:
		void update_insert_offset(std::u32string::iterator new_pos)
		{ update_insert_offset(std::distance(std::begin(m_value), new_pos)); }

		void update_insert_offset(size_t new_pos)
		{ m_insert_offset = new_pos; }

		move_only_function<void(single_line_text_input&, main::window_ref, main::ui_controller)> m_on_value_changed =
			move_only_function<void(single_line_text_input&, main::window_ref, main::ui_controller)>{no_operation_tag{}};

		std::u32string m_value;
		size_t m_insert_offset = 0;
		font_handling::glyph_sequence m_glyphs;
		basic_image<uint8_t> m_rendered_text;
		static constexpr auto text_dirty = 0x1;
		static constexpr auto host_textures_dirty = 0x2;
		static constexpr auto gpu_textures_dirty = 0x4;
		unsigned int m_dirty_bits = text_dirty | host_textures_dirty | gpu_textures_dirty;
		float m_margin = 0;
		unsigned int m_border_thickness = 0;
		std::shared_ptr<font_handling::font const> m_font;
		rgba_pixel m_bg_tint;
		rgba_pixel m_sel_tint;
		rgba_pixel m_fg_tint;

		main::generic_unique_texture m_background;
		// TODO: Move frame from foreground texture to frame texture
		main::generic_unique_texture m_foreground;
		// TODO: Frame should be should show the current frame
		main::generic_shared_texture m_frame;
		// TODO: Generate depending on what has been selected
		main::generic_shared_texture m_selection_mask;

		main::fb_size m_current_size;
		std::optional<std::u8string> m_placeholder;
		image m_background_host;
		image m_foreground_host;
	};
}

#endif
