#ifndef TERRAFORMER_UI_WIDGETS_BUTTON_HPP
#define TERRAFORMER_UI_WIDGETS_BUTTON_HPP

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
			if(m_current_stage == render_stage::update_texture) [[unlikely]]
			{
				{
					auto const background_intensity = (render_resources/"ui"/"command_area"/"background_intensity").get_if<float const>();
					assert(background_intensity != nullptr);
					auto const& descriptor = m_background_released.descriptor();
					auto const val = *background_intensity;
					auto const img = generate(
						drawing_api::beveled_rectangle{
							.width = static_cast<uint32_t>(descriptor.width),
							.height = static_cast<uint32_t>(descriptor.height),
							.border_thickness = m_border_thickness,
							.upper_left_color = rgba_pixel{val, val, val, 1.0f},
							.lower_right_color = 0.25f*rgba_pixel{val, val, val, 4.0f},
							.fill_color = 0.5f*rgba_pixel{val, val, val, 2.0f}
						}
					);
					m_background_released.upload(std::as_const(img).pixels(), descriptor.num_mipmaps);
				}

				{
					auto const background_intensity = (render_resources/"ui"/"command_area"/"background_intensity").get_if<float const>();
					assert(background_intensity != nullptr);
					auto const& descriptor = m_background_released.descriptor();
					auto const val = *background_intensity;
					auto const img = generate(
						drawing_api::beveled_rectangle{
							.width = static_cast<uint32_t>(descriptor.width),
							.height = static_cast<uint32_t>(descriptor.height),
							.border_thickness = m_border_thickness,
							.upper_left_color = 0.25f*rgba_pixel{val, val, val, 4.0f},
							.lower_right_color = rgba_pixel{val, val, val, 1.0f},
							.fill_color = 0.5f*rgba_pixel{val, val, val, 2.0f}
						}
					);
					m_background_pressed.upload(std::as_const(img).pixels(), descriptor.num_mipmaps);
				}

				{
					// TODO: Write helper function
					auto const& descriptor = m_foreground.descriptor();
					auto const w = static_cast<uint32_t>(descriptor.width);
					auto const h = static_cast<uint32_t>(descriptor.height);
					image img{w, h};
					for(uint32_t y = m_margin; y != h - m_margin; ++y)
					{
						for(uint32_t x = m_margin; x != w - m_margin; ++x)
						{
							auto const mask_val = static_cast<float>(m_rendered_text(x - m_margin, y - m_margin))/255.0f;
							img(x, y) = rgba_pixel{mask_val, mask_val, mask_val, mask_val};
						}
					}
					m_foreground.upload(std::as_const(img).pixels(), descriptor.num_mipmaps);
				}
				m_current_stage = render_stage::completed;
			}

			output_rect.foreground = &m_foreground;
			output_rect.background = (m_state == state::released)?
				&m_background_released : &m_background_pressed;
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
				m_current_stage = render_stage::update_texture;
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
		{
			drawing_api::gl_texture_descriptor descriptor{
				.width = size.width,
				.height = size.height,
				.format = drawing_api::to_gl_color_channel_layout_v<rgba_pixel>,
				.type = drawing_api::to_gl_type_id_v<rgba_pixel>,
				.num_mipmaps = 1
			};

			m_background_released.set_format(descriptor);
			m_background_pressed.set_format(descriptor);
			m_foreground.set_format(descriptor);
			m_current_stage = render_stage::update_texture;
		}

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
		enum class render_stage: int{update_text, update_texture, completed};
		mutable render_stage m_current_stage = render_stage::update_text;
		mutable unsigned int m_margin = 0;
		mutable unsigned int m_border_thickness = 0;

		drawing_api::gl_texture m_background_released;
		drawing_api::gl_texture m_background_pressed;
		drawing_api::gl_texture m_foreground;

		state m_value = state::released;
		state m_state = state::released;
		state m_saved_state = state::released;
	};
}

#endif