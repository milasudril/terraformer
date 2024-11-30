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
			.domain_size = span_2d_extents {
				.width = static_cast<uint32_t>(m_current_size.width),
				.height = static_cast<uint32_t>(m_current_size.height)
			},
			.origin_x = 0u,
			.origin_y = 0u,
			.width = static_cast<uint32_t>(m_current_size.width),
			.height = static_cast<uint32_t>(m_current_size.height),
			.border_thickness = m_border_thickness,
			.border_color = m_fg_tint,
			.fill_color = m_bg_tint
		}
	);

	m_foreground_host = drawing_api::convert_mask(m_rendered_text);

	m_input_marker_host = generate(
		drawing_api::flat_rectangle{
			.domain_size = span_2d_extents {
				.width = static_cast<uint32_t>(m_current_size.width),
				.height = static_cast<uint32_t>(m_current_size.height)
			},
			.origin_x = 0u,
			.origin_y = 0u,
			.width = 1u,
			.height = static_cast<uint32_t>(std::max(1.0f, static_cast<float>(m_current_size.height) - 2.0f*m_margin)),
			.border_thickness = 0u,
			.border_color = rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
			.fill_color = m_fg_tint
		}
	);

	// TODO: Selection mask should be grayscale
	auto const sel_begin = horz_offset_from_index(m_glyphs, m_sel_range.begin());
	// Add one to get inclusive boundaries
	auto const sel_end = horz_offset_from_index(m_glyphs, m_sel_range.end()) + 1.0f;

	printf("begin = %zu, end = %zu, cursor = %zu\n", m_sel_range.begin(), m_sel_range.end(), m_insert_offset);
	printf("sel_begin = %.8g, sel_end = %.8g\n", sel_begin, sel_end);

	m_selection_mask_host = generate(
		drawing_api::flat_rectangle{
			.domain_size = span_2d_extents {
				.width = static_cast<uint32_t>(m_current_size.width),
				.height = static_cast<uint32_t>(m_current_size.height)
			},
			.origin_x = static_cast<uint32_t>(sel_begin + m_margin),
			.origin_y = static_cast<uint32_t>(m_margin),
			.width = static_cast<uint32_t>(std::max(sel_end - sel_begin, 1.0f) + 0.5f),
			.height = static_cast<uint32_t>(
				std::max(static_cast<int32_t>(m_current_size.height) - static_cast<int32_t>(2.0f*m_margin), 1)
			),
			.border_thickness = 0u,
			.border_color = rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
			.fill_color = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f}
		}
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

	auto const cursor_loc = horz_offset_from_index(m_glyphs, m_insert_offset);

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
		(void)output_rect.set_bg_layer_mask(m_selection_mask.get());
		m_dirty_bits |= gpu_textures_dirty;
	}

	std::array const sel_tints{m_sel_tint, m_sel_tint, m_sel_tint, m_sel_tint};
	(void)output_rect.set_selection_background(m_background.get(), sel_tints);

	std::array const fg_tints{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint};
	auto const w_max = static_cast<float>(m_background_host.width()) - 2.0f*m_margin;
	displacement const input_marker_offset{std::min(cursor_loc, w_max) + m_margin , m_margin, 0.0f};
	auto const horz_fg_offset = cursor_loc >= w_max?
		w_max + m_margin - cursor_loc: m_margin;
	displacement const fg_offset{horz_fg_offset, m_margin, 0.0f};
	if(
		output_rect.set_widget_foreground(
			m_foreground.get(),
			fg_tints,
			fg_offset
		) != main::set_texture_result::success
	) [[unlikely]]
	{
		m_foreground = output_rect.create_texture();
		(void)output_rect.set_widget_foreground(m_foreground.get(), fg_tints, fg_offset);
		m_dirty_bits |= gpu_textures_dirty;
	}

	std::array const input_marker_tints{
		m_fg_tint*m_cursor_intensity,
		m_fg_tint*m_cursor_intensity,
		m_fg_tint*m_cursor_intensity,
		m_fg_tint*m_cursor_intensity
	};
	if(
		output_rect.set_input_marker(
			m_input_marker.get(),
			input_marker_tints,
			input_marker_offset
		) != main::set_texture_result::success
	) [[unlikely]]
	{
		m_input_marker = output_rect.create_texture();
		(void)output_rect.set_input_marker(m_input_marker.get(), input_marker_tints, input_marker_offset);
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
		m_input_marker.upload(std::as_const(m_input_marker_host).pixels());
		m_selection_mask.upload(std::as_const(m_selection_mask_host).pixels());
		m_dirty_bits &= ~gpu_textures_dirty;
	}
}

void terraformer::ui::widgets::single_line_text_input::handle_event(main::keyboard_button_event const& event, main::window_ref window, main::ui_controller)
{
	switch(to_builtin_command_id(event))
	{
		case main::builtin_command_id::go_to_begin:
			clear_selection();
			update_insert_offset(0);
			return;

		case main::builtin_command_id::select_to_begin:
			select_from_cursor_to_begin();
			update_insert_offset(0);
			return;

		case main::builtin_command_id::go_to_end:
			clear_selection();
			update_insert_offset(std::size(m_value));
			return;

		case main::builtin_command_id::select_to_end:
			select_from_cursor_to_end();
			update_insert_offset(std::size(m_value));
			return;

		case main::builtin_command_id::erase_backwards:
			if(!m_sel_range.empty())
			{ erase_selected_range(); }
			else
			if(!m_value.empty() && m_insert_offset != 0)
			{
				update_insert_offset(m_value.erase(std::begin(m_value) + m_insert_offset - 1));
				m_dirty_bits |= text_dirty;
			}
			return;

		case main::builtin_command_id::erase_forwards:
			if(!m_sel_range.empty())
			{ erase_selected_range(); }
			else
			if(m_insert_offset != std::size(m_value))
			{
				m_value.erase(std::begin(m_value) + m_insert_offset);
				m_dirty_bits |= text_dirty;
			}
			return;

		case main::builtin_command_id::step_left:
			clear_selection();
			clamped_decrement(m_insert_offset, 0);
			return;

		case main::builtin_command_id::select_left:
			step_selection_left();
			clamped_decrement(m_insert_offset, 0);
			return;

		case main::builtin_command_id::step_right:
			clear_selection();
			clamped_increment(m_insert_offset, std::size(m_value));
			return;

		case main::builtin_command_id::select_right:
			step_selection_right();
			clamped_increment(m_insert_offset, std::size(m_value));
			return;

		case main::builtin_command_id::step_up:
			clear_selection();
			return;

		case main::builtin_command_id::step_down:
			clear_selection();
			return;

		case main::builtin_command_id::select_all:
			select_all();
			return;

		case main::builtin_command_id::paste:
			if(!m_sel_range.empty())
			{ erase_selected_range(); }
			insert_at_cursor(window.get_clipboard_string().c_str());
			return;

		case main::builtin_command_id::copy:
			if(!m_sel_range.empty())
			{ window.set_clipboard_string(get_selection()); }
			return;

		case main::builtin_command_id::cut:
			if(!m_sel_range.empty())
			{
				window.set_clipboard_string(get_selection());
				erase_selected_range();
			}
			return;

		default:
			return;
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
			static_cast<float>(temp.width()) + 2.0f*m_margin,
			static_cast<float>(temp.height()) + 2.0f*m_margin,
			1.0f
		};
	}

	// TODO: Use height to find required width (multi-line)
	if(m_dirty_bits & text_dirty)
	{ regenerate_text_mask(); }

	return scaling{
		static_cast<float>(m_rendered_text.width()) + 2.0f*m_margin,
		static_cast<float>(m_rendered_text.height()) + 2.0f*m_margin,
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
			static_cast<float>(temp.width()) + 2.0f*m_margin,
			static_cast<float>(temp.height()) + 2.0f*m_margin,
			1.0f
		};
	}

	if(m_dirty_bits & text_dirty)
	{ regenerate_text_mask(); }

	return scaling{
		static_cast<float>(m_rendered_text.width()) + 2.0f*m_margin,
		static_cast<float>(m_rendered_text.height()) + 2.0f*m_margin,
		1.0f
	};
}

void terraformer::ui::widgets::single_line_text_input::theme_updated(main::config const& cfg, main::widget_instance_info)
{
	m_margin = cfg.input_area.padding + cfg.input_area.border_thickness;
	m_font = cfg.input_area.font;
	m_bg_tint = cfg.input_area.colors.background;
	m_sel_tint = cfg.input_area.colors.selection;
	m_fg_tint = cfg.input_area.colors.foreground;
	m_frame = cfg.misc_textures.null;
	m_border_thickness = static_cast<uint32_t>(cfg.input_area.border_thickness);
	m_dirty_bits |= host_textures_dirty | text_dirty;
}