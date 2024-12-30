//@	{"target":{"name":"single_line_text_input.o"}}

#include "./single_line_text_input.hpp"
#include "ui/drawing_api/image_generators.hpp"

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

	m_frame = generate(
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
			.border_color = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
			.fill_color = rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f}
		}
	);

	m_foreground = drawing_api::convert_mask(m_rendered_text);

	m_input_marker = generate(
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
			.fill_color = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f}
		}
	);

	// TODO: Selection mask should be grayscale
	auto const sel_begin = horz_offset_from_index(m_glyphs, m_sel_range.begin());
	// Add one to get inclusive boundaries
	auto const sel_end = horz_offset_from_index(m_glyphs, m_sel_range.end()) + 1.0f;

	printf("begin = %zu, end = %zu, cursor = %zu\n", m_sel_range.begin(), m_sel_range.end(), m_insert_offset);
	printf("sel_begin = %.8g, sel_end = %.8g\n", sel_begin, sel_end);

	m_selection_mask = generate(
		drawing_api::flat_rectangle{
			.domain_size = span_2d_extents {
				.width = static_cast<uint32_t>(m_rendered_text.width()),
				.height = static_cast<uint32_t>(m_rendered_text.height())
			},
			.origin_x = static_cast<uint32_t>(sel_begin),
			.origin_y = 0u,
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
}

terraformer::ui::main::widget_layer_stack
terraformer::ui::widgets::single_line_text_input::prepare_for_presentation(main::graphics_backend_ref backend)
{
	if(m_dirty_bits & text_dirty) [[unlikely]]
	{ regenerate_text_mask(); }

	// TODO: Only regenerate relevant host textures (frame only needs to be updated on resize)
	if(m_dirty_bits & host_textures_dirty) [[unlikely]]
	{ regenerate_textures(); }

	auto const cursor_loc = horz_offset_from_index(m_glyphs, m_insert_offset);

	auto const w_max = static_cast<float>(m_frame.frontend_resource().width()) - 2.0f*m_margin;
	displacement const input_marker_offset{std::min(cursor_loc, w_max) + m_margin , m_margin, 0.0f};
	auto const horz_fg_offset = cursor_loc >= w_max?
		w_max + m_margin - cursor_loc: m_margin;
	displacement const fg_offset{horz_fg_offset, m_margin, 0.0f};

	std::array const input_marker_tints{
			m_fg_tint*m_cursor_intensity,
			m_fg_tint*m_cursor_intensity,
			m_fg_tint*m_cursor_intensity,
			m_fg_tint*m_cursor_intensity
		};

	return main::widget_layer_stack{
		.background = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = m_background->get_backend_resource(backend).get(),
			.tints = std::array{m_bg_tint, m_bg_tint, m_bg_tint, m_bg_tint}
		},
		// FIXME: Selection mask is only correct when text fits in textbox
		.sel_bg_mask = main::widget_layer_mask{
			.offset = displacement{},
			.texture = m_selection_mask.get_backend_resource(backend).get()
		},
		.selection_background = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = m_background->get_backend_resource(backend).get(),
			.tints = std::array{m_sel_tint, m_sel_tint, m_sel_tint, m_sel_tint}
		},
		.foreground = main::widget_layer{
			.offset = fg_offset,
			.rotation = geosimd::turn_angle{},
			.texture = m_foreground.get_backend_resource(backend).get(),
			.tints = std::array{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint}
		},
		.frame = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = m_frame.get_backend_resource(backend).get(),
			.tints = std::array{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint}
		},
		.input_marker = main::widget_layer{
			.offset = input_marker_offset,
			.rotation = geosimd::turn_angle{},
			.texture = m_input_marker.get_backend_resource(backend).get(),
			.tints = input_marker_tints
		}
	};
}

void terraformer::ui::widgets::single_line_text_input::handle_event(main::keyboard_button_event const& event, main::window_ref window, main::ui_controller controller)
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
			m_on_value_changed(*this, window, controller);
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
			m_on_value_changed(*this, window, controller);
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
			m_on_step_up(*this, window, controller);
			clear_selection();
			return;

		case main::builtin_command_id::step_down:
			m_on_step_down(*this, window, controller);
			clear_selection();
			return;

		case main::builtin_command_id::select_all:
			select_all();
			return;

		case main::builtin_command_id::paste:
			if(!m_sel_range.empty())
			{ erase_selected_range(); }
			insert_at_cursor(window.get_clipboard_string().c_str());
			m_on_value_changed(*this, window, controller);
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
				m_on_value_changed(*this, window, controller);
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
		if(m_dirty_bits & recompute_size)
		{
			font_handling::text_shaper shaper{};

			// TODO: Add support for different scripts, direction, and languages
			// TODO: DRY
			auto result = shaper.append(*m_placeholder)
				.with(hb_script_t::HB_SCRIPT_LATIN)
				.with(hb_direction_t::HB_DIRECTION_LTR)
				.with(hb_language_from_string("en-UE", -1))
				.run(*m_font);

			auto const temp = render(result);
			m_dirty_bits &= ~recompute_size;
			m_widget_size = scaling{
				static_cast<float>(temp.width()) + 2.0f*m_margin,
				static_cast<float>(temp.height()) + 2.0f*m_margin,
				1.0f
			};
			return m_widget_size;
		}
		return m_widget_size;
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
	m_background = cfg.misc_textures.white;
	m_border_thickness = static_cast<uint32_t>(cfg.input_area.border_thickness);
	m_dirty_bits |= host_textures_dirty | text_dirty | recompute_size;
}