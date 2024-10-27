//@	{"dependencies_extra":[{"ref":"./single_line_text_input.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_SINGLE_LINE_TEXT_INPUT_HPP
#define TERRAFORMER_UI_WIDGETS_SINGLE_LINE_TEXT_INPUT_HPP

#include "ui/main/generic_texture.hpp"
#include "ui/drawing_api/image_generators.hpp"
#include "ui/font_handling/text_shaper.hpp"
#include "ui/main/widget.hpp"
#include "lib/common/object_tree.hpp"
#include "lib/common/move_only_function.hpp"

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
			m_value = std::forward<StringType>(new_val);
			m_dirty_bits |= text_dirty;
			return *this;
		}

		auto const& value() const
		{ return m_value; }

		void handle_event(main::typing_event event, main::window_ref, main::ui_controller)
		{
				m_value.push_back(event.codepoint);
				m_dirty_bits |= text_dirty;
		}

		void handle_event(main::keyboard_button_event const& event, main::window_ref, main::ui_controller)
		{
			if(
				event.scancode == 0xe &&
				(event.action == main::keyboard_button_action::press || event.action == main::keyboard_button_action::repeat)
			)
			{
				// TODO: Erase to the left of text cursor
				if(!m_value.empty())
				{
					m_value.pop_back();
					m_dirty_bits |= text_dirty;
				}
			}
		}

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
		move_only_function<void(single_line_text_input&, main::window_ref, main::ui_controller)> m_on_value_changed =
			move_only_function<void(single_line_text_input&, main::window_ref, main::ui_controller)>{no_operation_tag{}};

		std::basic_string<char32_t> m_value;
		basic_image<uint8_t> m_rendered_text;
		static constexpr auto text_dirty = 0x1;
		static constexpr auto host_textures_dirty = 0x2;
		static constexpr auto gpu_textures_dirty = 0x4;
		unsigned int m_dirty_bits = text_dirty | host_textures_dirty | gpu_textures_dirty;
		unsigned int m_margin = 0;
		unsigned int m_border_thickness = 0;
		std::shared_ptr<font_handling::font const> m_font;
		rgba_pixel m_bg_tint;
		rgba_pixel m_fg_tint;

		main::generic_unique_texture m_background;
		main::generic_unique_texture m_foreground;

		main::fb_size m_current_size;
		image m_background_host;
		image m_foreground_host;
	};

	inline void single_line_text_input::prepare_for_presentation(main::widget_rendering_result output_rect)
	{
		if(m_dirty_bits & host_textures_dirty) [[unlikely]]
		{ regenerate_textures(); }

		if(output_rect.set_foreground(m_foreground.get()) != main::set_texture_result::success) [[unlikely]]
		{
			m_foreground = output_rect.create_texture();
			(void)output_rect.set_foreground(m_foreground.get());
			m_dirty_bits |= gpu_textures_dirty;
		}
;
		if(
			output_rect.set_background(m_background.get())!=main::set_texture_result::success
		) [[unlikely]]
		{
			m_background = output_rect.create_texture();
			output_rect.set_background(m_background.get());
			m_dirty_bits |= gpu_textures_dirty;
		}

		if(m_dirty_bits & gpu_textures_dirty)
		{
			m_background.upload(std::as_const(m_background_host).pixels());
			m_foreground.upload(std::as_const(m_foreground_host).pixels());
			m_dirty_bits &= ~gpu_textures_dirty;
		}

		output_rect.set_background_tints(std::array{m_bg_tint, m_bg_tint, m_bg_tint, m_bg_tint});
		output_rect.set_foreground_tints(std::array{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint});
	}
}

#endif
