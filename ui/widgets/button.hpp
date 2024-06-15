#ifndef TERRAFORMER_UI_WIDGETS_BUTTON_HPP
#define TERRAFORMER_UI_WIDGETS_BUTTON_HPP

#include "./generic_texture.hpp"
#include "ui/drawing_api/single_quad_renderer.hpp"
#include "ui/drawing_api/image_generators.hpp"
#include "ui/font_handling/text_shaper.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"

namespace terraformer::ui::widgets
{
	class button
	{
	public:
		enum class state{released, pressed};

		void prepare_for_presentation(
			drawing_api::single_quad_renderer::input_rectangle& output_rect,
			main::widget_instance_info const&,
			object_dict const& render_resources
		)
		{
			if(m_current_stage == render_stage::regenerate_textures) [[unlikely]]
			{
				{
					auto const background_intensity = (render_resources/"ui"/"command_area"/"background_intensity").get_if<float const>();
					assert(background_intensity != nullptr);
					auto const val = *background_intensity;
					m_background_released_host = generate(
						drawing_api::beveled_rectangle{
							.width = static_cast<uint32_t>(m_current_size.width),
							.height = static_cast<uint32_t>(m_current_size.height),
							.border_thickness = m_border_thickness,
							.upper_left_color = rgba_pixel{val, val, val, 1.0f},
							.lower_right_color = 0.25f*rgba_pixel{val, val, val, 4.0f},
							.fill_color = 0.5f*rgba_pixel{val, val, val, 2.0f}
						}
					);
				}

				{
					auto const background_intensity = (render_resources/"ui"/"command_area"/"background_intensity").get_if<float const>();
					assert(background_intensity != nullptr);
					auto const val = *background_intensity;
					m_background_pressed_host = generate(
						drawing_api::beveled_rectangle{
							.width = static_cast<uint32_t>(m_current_size.width),
							.height = static_cast<uint32_t>(m_current_size.height),
							.border_thickness = m_border_thickness,
							.upper_left_color = 0.25f*rgba_pixel{val, val, val, 4.0f},
							.lower_right_color = rgba_pixel{val, val, val, 1.0f},
							.fill_color = 0.5f*rgba_pixel{val, val, val, 2.0f}
						}
					);
				}

				{
					// TODO: Write helper function
					auto const w = static_cast<uint32_t>(m_current_size.width);
					auto const h = static_cast<uint32_t>(m_current_size.height);
					m_foreground_host = image{w, h};
					for(uint32_t y = m_margin; y != h - m_margin; ++y)
					{
						for(uint32_t x = m_margin; x != w - m_margin; ++x)
						{
							auto const mask_val = static_cast<float>(m_rendered_text(x - m_margin, y - m_margin))/255.0f;
							m_foreground_host(x, y) = rgba_pixel{mask_val, mask_val, mask_val, mask_val};
						}
					}
				}

				m_current_stage = render_stage::completed;
			}

			if(!m_background_released)
			{
				m_background_released = output_rect.create_texture();
				m_background_released.upload(std::as_const(m_background_released_host).pixels());
				m_background_pressed = output_rect.create_texture();
				m_background_pressed.upload(std::as_const(m_background_pressed_host).pixels());
				m_foreground = output_rect.create_texture();
				m_foreground.upload(std::as_const(m_foreground_host).pixels());
			}

			output_rect.foreground = m_foreground.get_const();
			output_rect.background = (m_state == state::released)?
				m_background_released.get() : m_background_pressed.get_const();
			auto const bg_tint = (render_resources/"ui"/"command_area"/"background_tint").get_if<rgba_pixel const>();
			auto const fg_tint = (render_resources/"ui"/"command_area"/"text_color").get_if<rgba_pixel const>();
			assert(bg_tint != nullptr);
			assert(fg_tint != nullptr);

			output_rect.background_tints = std::array{*bg_tint, *bg_tint, *bg_tint, *bg_tint};
			output_rect.foreground_tints = std::array{*fg_tint, *fg_tint, *fg_tint, *fg_tint};
		}

		void handle_event(wsapi::cursor_enter_leave_event const& cle)
		{
			switch(cle.direction)
			{
				case wsapi::cursor_enter_leave::leave:
					m_saved_state = m_state;
					m_state = m_value;
					break;

				case wsapi::cursor_enter_leave::enter:
					m_state = m_saved_state;
					break;
			}
		}

		bool handle_event(wsapi::cursor_motion_event const&)
		{
			return false;
		}

		bool handle_event(wsapi::mouse_button_event const& mbe)
		{
			if(mbe.button == 0)
			{
				switch(mbe.action)
				{
					case wsapi::button_action::press:
						m_state = state::pressed;
						break;

					case wsapi::button_action::release:
						m_state = m_value;
						break;
				}
			}

			return false;
		}

		main::widget_size_constraints get_size_constraints(object_dict const& resources) const
		{
			if(m_current_stage == render_stage::update_text) [[unlikely]]
			{
				auto const font = (resources/"ui"/"command_area"/"font").get_if<font_handling::font const>();
				assert(font != nullptr);

				font_handling::text_shaper shaper{};

				// TODO: Add support for different scripts, direction, and languages
				auto result = shaper.append(m_text)
					.with(hb_script_t::HB_SCRIPT_LATIN)
					.with(hb_direction_t::HB_DIRECTION_LTR)
					.with(hb_language_from_string("en-UE", -1))
					.run(*font);

				m_rendered_text = render(result);
				m_current_stage = render_stage::regenerate_textures;
			}

			auto const margin = (resources/"ui"/"widget_inner_margin").get_if<unsigned int const>();
			auto const border_thickness = (resources/"ui"/"3d_border_thickness").get_if<unsigned int const>();
			assert(margin != nullptr);
			assert(border_thickness != nullptr);
			m_margin = *margin + *border_thickness;
			m_border_thickness = *border_thickness;

			return main::widget_size_constraints{
				.width{
					.min = static_cast<float>(m_rendered_text.width() + 2*m_margin),
					.max = std::numeric_limits<float>::infinity()
				},
				.height{
					.min = static_cast<float>(m_rendered_text.height() + 2*m_margin),
					.max = std::numeric_limits<float>::infinity()
				},
				.aspect_ratio = std::nullopt
			};
		}

		void handle_event(wsapi::fb_size size)
		{ m_current_size = size; }

		template<class StringType>
		button& text(StringType&& text)
		{
			m_text = std::forward<StringType>(text);
			m_current_stage = render_stage::update_text;
			return *this;
		}

		button& value(bool new_value)
		{
			m_value = new_value? state::pressed : state::released;
			return *this;
		}

		bool value() const
		{ return m_value == state::pressed; }

	private:
		std::basic_string<char8_t> m_text;
		mutable basic_image<uint8_t> m_rendered_text;
		enum class render_stage: int{update_text, regenerate_textures, completed};
		mutable render_stage m_current_stage = render_stage::update_text;
		mutable unsigned int m_margin = 0;
		mutable unsigned int m_border_thickness = 0;

		generic_unique_texture m_background_released;
		generic_unique_texture m_background_pressed;
		generic_unique_texture m_foreground;

		wsapi::fb_size m_current_size;
		image m_background_released_host;
		image m_background_pressed_host;
		image m_foreground_host;

		state m_value = state::released;
		state m_state = state::released;
		state m_saved_state = state::released;
	};
}

#endif
