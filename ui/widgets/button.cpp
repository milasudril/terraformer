//@	{"target":{"name":"button.o"}}

#include "./button.hpp"

void terraformer::ui::widgets::button::regenerate_text_mask()
{
	font_handling::text_shaper shaper{};

	// TODO: Add support for different scripts, direction, and languages
	auto result = shaper.append(m_text)
		.with(hb_script_t::HB_SCRIPT_LATIN)
		.with(hb_direction_t::HB_DIRECTION_LTR)
		.with(hb_language_from_string("en-UE", -1))
		.run(*m_font);

	m_rendered_text = render(result);
	m_dirty_bits &= ~text_dirty;
	m_dirty_bits |= host_textures_dirty;
}

void terraformer::ui::widgets::button::regenerate_textures()
{
	if(m_dirty_bits & text_dirty) [[unlikely]]
	{ regenerate_text_mask(); }

	m_background_released_host = generate(
		drawing_api::beveled_rectangle{
			.width = static_cast<uint32_t>(m_current_size.width),
			.height = static_cast<uint32_t>(m_current_size.height),
			.border_thickness = m_border_thickness,
			.upper_left_color = m_bg_tint,
			.lower_right_color = 0.25f*m_bg_tint + rgba_pixel{0.0f, 0.0f, 0.0f, 0.75f},
			.fill_color = 0.5f*m_bg_tint + rgba_pixel{0.0f, 0.0f, 0.0f, 0.5f},
		}
	);

	m_background_pressed_host = generate(
		drawing_api::beveled_rectangle{
			.width = static_cast<uint32_t>(m_current_size.width),
			.height = static_cast<uint32_t>(m_current_size.height),
			.border_thickness = m_border_thickness,
			.upper_left_color = 0.25f*m_bg_tint + rgba_pixel{0.0f, 0.0f, 0.0f, 0.75f},
			.lower_right_color = m_bg_tint,
			.fill_color = 0.5f*m_bg_tint + rgba_pixel{0.0f, 0.0f, 0.0f, 0.5f},
		}
	);

	m_foreground_host = drawing_api::convert_mask(m_rendered_text);

	m_dirty_bits &= ~host_textures_dirty;
	m_dirty_bits |= gpu_textures_dirty;
}

void terraformer::ui::widgets::button::prepare_for_presentation(main::widget_rendering_result output_rect)
{
	auto const display_state = m_temp_state.value_or(m_value);

	if(m_dirty_bits & host_textures_dirty) [[unlikely]]
	{ regenerate_textures(); }


	std::array const fg_tint{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint};
	displacement const fg_offset{m_margin, m_margin, 0.0f};
	if(
		output_rect.set_widget_foreground(
			m_foreground.get(),
			fg_tint,
			fg_offset
		) != main::set_texture_result::success
	) [[unlikely]]
	{
		m_foreground = output_rect.create_texture();
		(void)output_rect.set_widget_foreground(m_foreground.get(), fg_tint, fg_offset);
		m_dirty_bits |= gpu_textures_dirty;
	}

	std::array const bg_tint{m_bg_tint, m_bg_tint, m_bg_tint, m_bg_tint};
	if(
		output_rect.set_widget_background(
			(
				display_state == state::released)?
					m_background_released.get() : m_background_pressed.get(),
				bg_tint
			) != main::set_texture_result::success
	) [[unlikely]]
	{
		m_background_released = output_rect.create_texture();
		m_background_pressed = output_rect.create_texture();
		output_rect.set_widget_background(
			(display_state == state::released)?
				m_background_released.get() : m_background_pressed.get(),
			bg_tint
		);
		m_dirty_bits |= gpu_textures_dirty;
	}

	if(m_dirty_bits & gpu_textures_dirty)
	{
		m_background_released.upload(std::as_const(m_background_released_host).pixels());
		m_background_pressed.upload(std::as_const(m_background_pressed_host).pixels());
		m_foreground.upload(std::as_const(m_foreground_host).pixels());
		m_dirty_bits &= ~gpu_textures_dirty;
	}

	output_rect.set_bg_layer_mask(m_null_texture.get());
	output_rect.set_selection_background(m_null_texture.get(), std::array<rgba_pixel, 4>{});	output_rect.set_frame(m_null_texture.get(), std::array<rgba_pixel, 4>{});
}

terraformer::scaling terraformer::ui::widgets::button::compute_size(main::widget_width_request)
{
	// TODO: Use height to find required width (multi-line)
	if(m_dirty_bits & text_dirty)
	{ regenerate_text_mask(); }

	return scaling{
		static_cast<float>(m_rendered_text.width()) + 2.0f*m_margin,
		static_cast<float>(m_rendered_text.height()) + 2.0f*m_margin,
		1.0f
	};
}

terraformer::scaling terraformer::ui::widgets::button::compute_size(main::widget_height_request)
{
	// TODO: Use width to find required height (multi-line)
	if(m_dirty_bits & text_dirty)
	{ regenerate_text_mask(); }

	return scaling{
		static_cast<float>(m_rendered_text.width()) + 2.0f*m_margin,
		static_cast<float>(m_rendered_text.height()) + 2.0f*m_margin,
		1.0f
	};
}

void terraformer::ui::widgets::button::theme_updated(main::config const& cfg, main::widget_instance_info)
{
	m_margin = cfg.command_area.padding + cfg.command_area.border_thickness;
	m_font = cfg.command_area.font;
	m_bg_tint = cfg.command_area.colors.background;
	m_fg_tint = cfg.command_area.colors.foreground;
	m_border_thickness = static_cast<uint32_t>(cfg.command_area.border_thickness);
	m_dirty_bits |= host_textures_dirty | text_dirty;
	m_null_texture = cfg.misc_textures.null;
}