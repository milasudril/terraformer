//@	{"dependencies_extra":[{"ref":"./button.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_BUTTON_HPP
#define TERRAFORMER_UI_WIDGETS_BUTTON_HPP

#include "ui/main/generic_texture.hpp"
#include "ui/drawing_api/image_generators.hpp"
#include "ui/font_handling/text_shaper.hpp"
#include "ui/main/widget.hpp"
#include "lib/common/object_tree.hpp"
#include "lib/common/move_only_function.hpp"

namespace terraformer::ui::widgets
{
	class button:public main::widget_with_default_actions
	{
	public:
		using widget_with_default_actions::handle_event;

		enum class state{released, pressed};

		template<class Function>
		button& on_activated(Function&& func)
		{
			m_on_activated = std::forward<Function>(func);
			return *this;
		}

		template<class StringType>
		button& text(StringType&& text)
		{
			m_text = std::forward<StringType>(text);
			m_dirty_bits |= text_dirty;
			return *this;
		}

		button& value(bool new_value)
		{
			m_value = new_value? state::pressed : state::released;
			return *this;
		}

		bool value() const
		{ return m_value == state::pressed; }

		void regenerate_text_mask();

		void regenerate_textures();

		void prepare_for_presentation(main::widget_rendering_result output_rect);

		void handle_event(main::cursor_enter_event const&)
		{ }

		void handle_event(main::cursor_leave_event const&)
		{ m_temp_state = std::nullopt; }

		void handle_event(main::cursor_motion_event const&)
		{ }

		void handle_event(main::mouse_button_event const& mbe)
		{
			if(mbe.button == 0)
			{
				switch(mbe.action)
				{
					case main::mouse_button_action::press:
						m_temp_state = state::pressed;
						break;

					case main::mouse_button_action::release:
						if(m_temp_state.has_value())
						{
							m_temp_state.reset();
							m_on_activated(*this);
						}
						break;
				}
			}
		}

		scaling compute_size(main::widget_width_request req);

		scaling compute_size(main::widget_height_request req);

		void handle_event(main::fb_size size)
		{
			m_current_size = size;
			m_dirty_bits |= host_textures_dirty;
		}

		void theme_updated(main::config const& cfg, main::widget_instance_info);

		main::layout_policy_ref get_layout() const
		{ return main::layout_policy_ref{}; }

		main::widget_collection_ref get_children()
		{ return main::widget_collection_ref{}; }

		main::widget_collection_view get_children() const
		{ return main::widget_collection_view{}; }

	private:
		move_only_function<void(button&)> m_on_activated =
			move_only_function<void(button&)>{no_operation_tag{}};

		std::basic_string<char8_t> m_text;
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

		main::generic_unique_texture m_background_released;
		main::generic_unique_texture m_background_pressed;
		main::generic_unique_texture m_foreground;

		main::fb_size m_current_size;
		image m_background_released_host;
		image m_background_pressed_host;
		image m_foreground_host;

		state m_value = state::released;
		std::optional<state> m_temp_state;
	};

	inline void button::prepare_for_presentation(main::widget_rendering_result output_rect)
	{
		auto const display_state = m_temp_state.value_or(m_value);

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
			output_rect.set_background( (display_state == state::released)?
			m_background_released.get() : m_background_pressed.get())!=main::set_texture_result::success
		) [[unlikely]]
		{
			m_background_released = output_rect.create_texture();
			m_background_pressed = output_rect.create_texture();
			output_rect.set_background((display_state == state::released)?
				m_background_released.get() : m_background_pressed.get());
			m_dirty_bits |= gpu_textures_dirty;
		}

		if(m_dirty_bits & gpu_textures_dirty)
		{
			m_background_released.upload(std::as_const(m_background_released_host).pixels());
			m_background_pressed.upload(std::as_const(m_background_pressed_host).pixels());
			m_foreground.upload(std::as_const(m_foreground_host).pixels());
			m_dirty_bits &= ~gpu_textures_dirty;
		}

		output_rect.set_background_tints(std::array{m_bg_tint, m_bg_tint, m_bg_tint, m_bg_tint});
		output_rect.set_foreground_tints(std::array{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint});
	}

	class toggle_button:private button
	{
	public:
		toggle_button()
		{
			button::on_activated(bound_callable<on_activated_callback>{});
		}

		using button::handle_event;
		using button::prepare_for_presentation;
		using button::text;
		using button::value;
		using button::theme_updated;
		using button::get_layout;
		using button::get_children;
		using button::compute_size;

		template<class Function>
		toggle_button& on_value_changed(Function&& func)
		{
			button::on_activated([cb = std::forward<Function>(func)](button& src){
				on_activated_callback(src);
				cb(static_cast<toggle_button&>(src));
			});
			return *this;
		}

		template<class StringType>
		toggle_button& text(StringType&& text)
		{
			button::text(std::forward<StringType>(text));
			return *this;
		}

		toggle_button& value(bool new_value)
		{
			button::value(new_value);
			return *this;
		}

	private:
		static void on_activated_callback(button& source)
		{ source.value(!source.value()); }
	};
}

#endif
