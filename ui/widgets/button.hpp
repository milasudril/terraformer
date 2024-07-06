//@	{"dependencies_extra":[{"ref":"./button.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_BUTTON_HPP
#define TERRAFORMER_UI_WIDGETS_BUTTON_HPP

#include "./generic_texture.hpp"
#include "ui/drawing_api/image_generators.hpp"
#include "ui/font_handling/text_shaper.hpp"
#include "ui/main/widget.hpp"
#include "lib/common/object_tree.hpp"
#include "lib/common/move_only_function.hpp"

namespace terraformer::ui::widgets
{
	class button
	{
	public:
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

		template<class OutputRectangle>
		void prepare_for_presentation(
			OutputRectangle& output_rect,
			main::widget_instance_info const&,
			object_dict const& render_resources
		);

		void handle_event(wsapi::cursor_enter_leave_event const& cle)
		{
			switch(cle.direction)
			{
				case wsapi::cursor_enter_leave::leave:
					m_temp_state = std::nullopt;
					break;

				case wsapi::cursor_enter_leave::enter:
					break;
			}
		}

		void handle_event(wsapi::cursor_motion_event const&, main::input_device_grab&)
		{ }

		void handle_event(wsapi::mouse_button_event const& mbe, main::input_device_grab&)
		{
			if(mbe.button == 0)
			{
				switch(mbe.action)
				{
					case wsapi::button_action::press:
						m_temp_state = state::pressed;
						break;

					case wsapi::button_action::release:
						m_temp_state.reset();
						m_on_activated(*this);
						break;
				}
			}
		}

		main::widget_size_constraints get_size_constraints() const;

		void handle_event(wsapi::fb_size size)
		{
			m_current_size = size;
			m_dirty_bits |= host_textures_dirty;
		}

		void theme_updated(object_dict const& render_resources);

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
		shared_const_any m_font;
		float m_background_intensity;

		generic_unique_texture m_background_released;
		generic_unique_texture m_background_pressed;
		generic_unique_texture m_foreground;

		wsapi::fb_size m_current_size;
		image m_background_released_host;
		image m_background_pressed_host;
		image m_foreground_host;

		state m_value = state::released;
		std::optional<state> m_temp_state;
	};

	template<class OutputRectangle>
	void button::prepare_for_presentation(
		OutputRectangle& output_rect,
		main::widget_instance_info const&,
		object_dict const& render_resources
	)
	{
		auto const display_state = m_temp_state.value_or(m_value);

		if(m_dirty_bits & host_textures_dirty) [[unlikely]]
		{ regenerate_textures(); }

		output_rect.foreground = m_foreground.get_const();
		if(!output_rect.foreground)
		{
			m_foreground = output_rect.create_texture();
			output_rect.foreground = m_foreground.get_const();
			m_dirty_bits |= gpu_textures_dirty;
		}

		output_rect.background = (display_state == state::released)?
			m_background_released.get() : m_background_pressed.get_const();
		if(!output_rect.background)
		{
			m_background_released = output_rect.create_texture();
			m_background_pressed = output_rect.create_texture();
			output_rect.background = (display_state == state::released)?
				m_background_released.get() : m_background_pressed.get_const();
			m_dirty_bits |= gpu_textures_dirty;
		}

		if(m_dirty_bits & gpu_textures_dirty)
		{
			m_background_released.upload(std::as_const(m_background_released_host).pixels());
			m_background_pressed.upload(std::as_const(m_background_pressed_host).pixels());
			m_foreground.upload(std::as_const(m_foreground_host).pixels());
			m_dirty_bits &= ~gpu_textures_dirty;
		}

		auto const bg_tint = (render_resources/"ui"/"command_area"/"background_tint").get_if<rgba_pixel const>();
		auto const fg_tint = (render_resources/"ui"/"command_area"/"text_color").get_if<rgba_pixel const>();
		assert(bg_tint != nullptr);
		assert(fg_tint != nullptr);

		output_rect.background_tints = std::array{*bg_tint, *bg_tint, *bg_tint, *bg_tint};
		output_rect.foreground_tints = std::array{*fg_tint, *fg_tint, *fg_tint, *fg_tint};
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
		using button::get_size_constraints;
		using button::text;
		using button::value;
		using button::theme_updated;

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
