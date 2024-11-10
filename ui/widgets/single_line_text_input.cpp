//@	{"target":{"name":"single_line_text_input.o"}}

#include "./single_line_text_input.hpp"

void terraformer::ui::widgets::single_line_text_input::regenerate_text_mask()
{
	font_handling::text_shaper shaper{};

	// TODO: Add support for different scripts, direction, and languages
	m_glyphs = shaper.append(m_value)
		.with(hb_script_t::HB_SCRIPT_LATIN)
		.with(hb_direction_t::HB_DIRECTION_LTR)
		.with(hb_language_from_string("en-UE", -1))
		.run(*m_font);

	m_rendered_text = render(m_glyphs);
	m_dirty_bits &= ~text_dirty;
	m_dirty_bits |= host_textures_dirty;
}

void terraformer::ui::widgets::single_line_text_input::regenerate_textures()
{
	if(m_dirty_bits & text_dirty) [[unlikely]]
	{ regenerate_text_mask(); }

	m_background_host = generate(
		drawing_api::flat_rectangle{
			.width = static_cast<uint32_t>(m_current_size.width),
			.height = static_cast<uint32_t>(m_current_size.height),
			.border_thickness = m_border_thickness,
			.border_color = m_fg_tint,
			.fill_color = m_bg_tint
		}
	);

	m_foreground_host = drawing_api::convert_mask(
		m_current_size.width,
		m_current_size.height,
		m_rendered_text,
		m_margin
	);

	m_dirty_bits &= ~host_textures_dirty;
	m_dirty_bits |= gpu_textures_dirty;
}

void terraformer::ui::widgets::single_line_text_input::prepare_for_presentation(main::widget_rendering_result output_rect)
{
	if(m_dirty_bits & text_dirty) [[unlikely]]
	{ regenerate_text_mask(); }

	// TODO: Only regenerate relevant host textures
	if(m_dirty_bits & host_textures_dirty) [[unlikely]]
	{ regenerate_textures(); }

	{
		// Compute cursor location

		if(m_insert_offset == 0)
		{
			printf("%.8g %.8g\n", 0.0f, 0.0f);
		}
		else
		{
			// FIXME: This only works if last glyph is not a ligature. Test with ff
			auto const geom = input_index_to_location(m_glyphs, m_insert_offset - 1)
				.value_or(font_handling::glyph_geometry{});
			auto cursor_loc = geom.loc + geom.advance;
			printf("%.8g %.8g\n", cursor_loc[0], cursor_loc[1]);
		}
	}

	std::array const bg_tints{m_bg_tint, m_bg_tint, m_bg_tint, m_bg_tint};
	if(output_rect.set_widget_background(m_background.get(), bg_tints) != main::set_texture_result::success) [[unlikely]]
	{
		m_background = output_rect.create_texture();
		output_rect.set_widget_background(m_background.get(), bg_tints);
		m_dirty_bits |= gpu_textures_dirty;
	}

	if(output_rect.set_bg_layer_mask(m_selection_mask.get()) != main::set_texture_result::success) [[unlikely]]
	{
		m_selection_mask = output_rect.create_texture();
		(void)output_rect.set_bg_layer_mask(m_foreground.get());
		m_dirty_bits |= gpu_textures_dirty;
	}

	std::array const sel_tints{m_sel_tint, m_sel_tint, m_sel_tint, m_sel_tint};
	(void)output_rect.set_selection_background(m_background.get(), sel_tints);

	std::array const fg_tints{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint};
	if(output_rect.set_widget_foreground(m_foreground.get(), fg_tints) != main::set_texture_result::success) [[unlikely]]
	{
		m_foreground = output_rect.create_texture();
		(void)output_rect.set_widget_foreground(m_foreground.get(), fg_tints);
		m_dirty_bits |= gpu_textures_dirty;
	}

	if(output_rect.set_frame(m_frame.get(), fg_tints) != main::set_texture_result::success) [[unlikely]]
	{
		m_frame = output_rect.create_texture();
		(void)output_rect.set_frame(m_frame.get(), fg_tints);
		m_dirty_bits |= gpu_textures_dirty;
	}

	// TODO: Only upload relevant textures
	if(m_dirty_bits & gpu_textures_dirty)
	{
		m_background.upload(std::as_const(m_background_host).pixels());
		m_foreground.upload(std::as_const(m_foreground_host).pixels());
		m_dirty_bits &= ~gpu_textures_dirty;
	}
}

void terraformer::ui::widgets::single_line_text_input::handle_event(main::keyboard_button_event const& event, main::window_ref, main::ui_controller)
{
	if(event.scancode == 0x66 && event.action == main::keyboard_button_action::press)
	{ update_insert_offset(0); }
	else
	if(event.scancode == 0x6b && event.action == main::keyboard_button_action::press)
	{ update_insert_offset(std::size(m_value)); }
	else
	if(
		event.action == main::keyboard_button_action::press
		|| event.action == main::keyboard_button_action::repeat
	)
	{
		if(event.scancode == 0xe)
		{
			if(!m_value.empty() && m_insert_offset != 0)
			{
				update_insert_offset(m_value.erase(std::begin(m_value) + m_insert_offset - 1));
				m_dirty_bits |= text_dirty;
			}
		}
		else
		if(event.scancode == 0x6f)
		{
			if(m_insert_offset != std::size(m_value))
			{
				m_value.erase(std::begin(m_value) + m_insert_offset);
				m_dirty_bits |= text_dirty;
			}
		}
		else
		if(event.scancode == 0x67)
		{ printf("Browse back\n"); }
		else
		if(event.scancode == 0x6c)
		{ printf("Browse forward\n"); }
		else
		if(event.scancode == 0x69)
		{ clamped_decrement(m_insert_offset, 0); }
		else
		if(event.scancode == 0x6a)
		{ clamped_increment(m_insert_offset, std::size(m_value)); }
		else
		{
			printf("%08x\n", event.scancode);
		}
	}
}

terraformer::scaling terraformer::ui::widgets::single_line_text_input::compute_size(main::widget_width_request)
{
	if(m_placeholder.has_value())
	{
		font_handling::text_shaper shaper{};

		// TODO: Add support for different scripts, direction, and languages
		// TODO: DRY
		// TODO: Cache result
		auto result = shaper.append(*m_placeholder)
			.with(hb_script_t::HB_SCRIPT_LATIN)
			.with(hb_direction_t::HB_DIRECTION_LTR)
			.with(hb_language_from_string("en-UE", -1))
			.run(*m_font);

		auto const temp = render(result);
		return scaling{
			static_cast<float>(temp.width() + 2*m_margin),
			static_cast<float>(temp.height() + 2*m_margin),
			1.0f
		};
	}

	// TODO: Use height to find required width (multi-line)
	if(m_dirty_bits & text_dirty)
	{ regenerate_text_mask(); }

	return scaling{
		static_cast<float>(m_rendered_text.width() + 2*m_margin),
		static_cast<float>(m_rendered_text.height() + 2*m_margin),
		1.0f
	};
}

terraformer::scaling terraformer::ui::widgets::single_line_text_input::compute_size(main::widget_height_request)
{
	if(m_placeholder.has_value())
	{
		font_handling::text_shaper shaper{};

		// TODO: Add support for different scripts, direction, and languages
		// TODO: DRY
		// TODO: Cache result
		auto result = shaper.append(*m_placeholder)
			.with(hb_script_t::HB_SCRIPT_LATIN)
			.with(hb_direction_t::HB_DIRECTION_LTR)
			.with(hb_language_from_string("en-UE", -1))
			.run(*m_font);

		auto const temp = render(result);
		return scaling{
			static_cast<float>(temp.width() + 2*m_margin),
			static_cast<float>(temp.height() + 2*m_margin),
			1.0f
		};
	}

	if(m_dirty_bits & text_dirty)
	{ regenerate_text_mask(); }

	return scaling{
		static_cast<float>(m_rendered_text.width() + 2*m_margin),
		static_cast<float>(m_rendered_text.height() + 2*m_margin),
		1.0f
	};
}

void terraformer::ui::widgets::single_line_text_input::theme_updated(main::config const& cfg, main::widget_instance_info)
{
	m_margin = static_cast<uint32_t>(cfg.input_area.padding + cfg.input_area.border_thickness);
	m_font = cfg.input_area.font;
	m_bg_tint = cfg.input_area.colors.background;
	m_sel_tint = cfg.input_area.colors.selection;
	m_fg_tint = cfg.input_area.colors.foreground;
	m_selection_mask = cfg.misc_textures.null;
	m_frame = cfg.misc_textures.null;
	m_border_thickness = static_cast<uint32_t>(cfg.input_area.border_thickness);
	m_dirty_bits |= host_textures_dirty | text_dirty;
}