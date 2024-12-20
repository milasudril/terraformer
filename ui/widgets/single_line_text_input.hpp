//@	{"dependencies_extra":[{"ref":"./single_line_text_input.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_SINGLE_LINE_TEXT_INPUT_HPP
#define TERRAFORMER_UI_WIDGETS_SINGLE_LINE_TEXT_INPUT_HPP

#include "ui/main/texture_types.hpp"
#include "ui/font_handling/text_shaper.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"

#include <type_traits>

namespace terraformer::ui::widgets
{
	class single_line_text_input:public main::widget_with_default_actions
	{
	public:
		class selection_range
		{
		public:
			explicit selection_range() = default;

			explicit selection_range(size_t begin, size_t end):m_begin{begin}, m_end{end}
			{
				if (begin > end)
				{ throw std::runtime_error{"Invalid selection range from"}; }
			}

			void extend_left()
			{ clamped_decrement(m_begin, 0); }

			void shrink_left()
			{ clamped_increment(m_begin, m_end); }

			void extend_right(size_t maxval)
			{ clamped_increment(m_end, maxval); }

			void shrink_right()
			{ clamped_decrement(m_end, m_begin); }

			bool empty() const { return m_begin == m_end; }

			size_t begin() const { return m_begin; }

			size_t end() const { return m_end; }

			bool operator==(selection_range const&) const = default;

			bool operator!=(selection_range const&) const = default;

			void clear()
			{ *this = selection_range{}; }

		private:
			size_t m_begin{0};
			size_t m_end{0};
		};

		using widget_with_default_actions::handle_event;

		void handle_event(main::keyboard_focus_enter_event, main::window_ref, main::ui_controller)
		{ m_cursor_intensity = 1.0f; }

		void handle_event(main::keyboard_focus_leave_event, main::window_ref, main::ui_controller)
		{
			// TODO: This should be read from ui config
			m_cursor_intensity = 0.6125f;
		}

		template<class Function>
		single_line_text_input& on_value_changed(Function&& func)
		{
			m_on_value_changed = std::forward<Function>(func);
			return *this;
		}

		template<class Function>
		single_line_text_input& on_step_up(Function&& func)
		{
			m_on_step_up = std::forward<Function>(func);
			return *this;
		}

		template<class Function>
		single_line_text_input& on_step_down(Function&& func)
		{
			m_on_step_down = std::forward<Function>(func);
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

		void handle_event(main::typing_event event, main::window_ref window, main::ui_controller controller)
		{
			if(!m_sel_range.empty())
			{ erase_selected_range(); }

			auto const i = std::begin(m_value) + m_insert_offset;
			update_insert_offset(m_value.insert(i, event.codepoint) + 1);
			m_dirty_bits |= text_dirty;
			m_on_value_changed(*this, window, controller);
		}

		void handle_event(main::keyboard_button_event const& event, main::window_ref, main::ui_controller);

		void regenerate_text_mask();

		void regenerate_textures();

		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend);

		scaling compute_size(main::widget_width_request req);

		scaling compute_size(main::widget_height_request req);

		void handle_event(main::fb_size size)
		{
			m_current_size = size;
			m_dirty_bits |= host_textures_dirty;
		}

		void theme_updated(main::config const& cfg, main::widget_instance_info);

		void step_selection_left()
		{
			if(m_sel_range.begin() == m_sel_range.end())
			{ m_sel_range = selection_range{m_insert_offset, m_insert_offset}; }

			if(m_sel_range.begin() == m_insert_offset)
			{ m_sel_range.extend_left(); }
			else
			{ m_sel_range.shrink_right(); }

			m_dirty_bits |= host_textures_dirty;
		}

		void step_selection_right()
		{
			if(m_sel_range.begin() == m_sel_range.end())
			{ m_sel_range = selection_range{m_insert_offset, m_insert_offset}; }

			if(m_sel_range.end() == m_insert_offset)
			{ m_sel_range.extend_right(std::size(m_value)); }
			else
			{ m_sel_range.shrink_left(); }

			m_dirty_bits |= host_textures_dirty;
		}

		void clear_selection()
		{
			m_sel_range.clear();
			m_dirty_bits |= host_textures_dirty;
		}

		void select_from_cursor_to_begin()
		{
			if(m_sel_range.begin() == m_insert_offset)
			{ m_sel_range = selection_range{0, m_sel_range.end()}; }
			else
			{ m_sel_range = selection_range{0, m_insert_offset}; }
			m_dirty_bits |= host_textures_dirty;
		}

		void select_from_cursor_to_end()
		{
			if(m_sel_range.end() == m_insert_offset)
			{ m_sel_range = selection_range{m_sel_range.begin(), std::size(m_value)}; }
			else
			{ m_sel_range = selection_range{m_insert_offset, std::size(m_value)}; }
			m_dirty_bits |= host_textures_dirty;
		}

		void erase_selected_range()
		{
			printf("Erasing range %zu %zu (size of buffer is %zu)\n", m_sel_range.begin(), m_sel_range.end(), std::size(m_value));
			m_value.erase(
				std::begin(m_value) + m_sel_range.begin(),
				std::begin(m_value) + m_sel_range.end()
			);
			m_insert_offset = m_sel_range.begin();
			m_sel_range.clear();
			m_dirty_bits |= text_dirty;
		}

		void select_all()
		{
			m_sel_range = selection_range{0, std::size(m_value)};
			m_insert_offset = std::size(m_value);
			m_dirty_bits |= host_textures_dirty;
		}

		void insert_at_cursor(std::u8string_view str)
		{
			auto val_temp = to_utf32(str);
			m_value.insert(m_insert_offset, val_temp);
			m_insert_offset += std::size(val_temp);
			m_dirty_bits |= text_dirty | host_textures_dirty;
		}

		std::u8string get_selection() const
		{
			return to_utf8(
				std::u32string_view{
					std::begin(m_value) + m_sel_range.begin(),
					std::begin(m_value) + m_sel_range.end()
				}
			);
		}

	private:
		void update_insert_offset(std::u32string::iterator new_pos)
		{ update_insert_offset(std::distance(std::begin(m_value), new_pos)); }

		void update_insert_offset(size_t new_pos)
		{ m_insert_offset = new_pos; }

		main::widget_action<single_line_text_input> m_on_value_changed = main::widget_action<single_line_text_input>{no_operation_tag{}};
		main::widget_action<single_line_text_input> m_on_step_up = main::widget_action<single_line_text_input>{no_operation_tag{}};
		main::widget_action<single_line_text_input> m_on_step_down = main::widget_action<single_line_text_input>{no_operation_tag{}};

		std::u32string m_value;
		size_t m_insert_offset = 0;
		selection_range m_sel_range{};

		font_handling::glyph_sequence m_glyphs;
		basic_image<uint8_t> m_rendered_text;
		// TODO: Review flags
		static constexpr auto text_dirty = 0x1;
		static constexpr auto host_textures_dirty = 0x2;
		unsigned int m_dirty_bits = text_dirty | host_textures_dirty;
		float m_margin = 0;
		unsigned int m_border_thickness = 0;
		std::shared_ptr<font_handling::font const> m_font;
		rgba_pixel m_bg_tint;
		rgba_pixel m_sel_tint;
		rgba_pixel m_fg_tint;
		float m_cursor_intensity = 0.6125f;

		main::immutable_shared_texture m_background;
		main::unique_texture m_selection_mask;
		main::immutable_shared_texture m_sel_background;
		main::unique_texture m_foreground;
		main::unique_texture m_frame;
		main::unique_texture m_input_marker;

		main::fb_size m_current_size;
		std::optional<std::u8string> m_placeholder;
	};
}

#endif
