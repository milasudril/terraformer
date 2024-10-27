//@	{"target":{"name":"single_line_text_input.o"}}

#include "./single_line_text_input.hpp"

void terraformer::ui::widgets::single_line_text_input::regenerate_text_mask()
{
	font_handling::text_shaper shaper{};

	// TODO: Add support for different scripts, direction, and languages
	auto result = shaper.append(m_value)
		.with(hb_script_t::HB_SCRIPT_LATIN)
		.with(hb_direction_t::HB_DIRECTION_LTR)
		.with(hb_language_from_string("en-UE", -1))
		.run(*m_font);

	m_rendered_text = render(result);
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
		static_cast<uint32_t>(m_current_size.width),
		static_cast<uint32_t>(m_current_size.height),
		m_rendered_text,
		m_margin
	);

	m_dirty_bits &= ~host_textures_dirty;
	m_dirty_bits |= gpu_textures_dirty;
}

terraformer::scaling terraformer::ui::widgets::single_line_text_input::compute_size(main::widget_width_request)
{
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
	m_fg_tint = cfg.input_area.colors.foreground;
	m_border_thickness = static_cast<uint32_t>(cfg.input_area.border_thickness);
	m_dirty_bits |= host_textures_dirty | text_dirty;
}