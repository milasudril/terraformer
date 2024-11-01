//@	{"target":{"name":"label.o"}}

#include "./label.hpp"

void terraformer::ui::widgets::label::regenerate_text_mask()
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

void terraformer::ui::widgets::label::regenerate_textures()
{
	if(m_dirty_bits & text_dirty)
	{ regenerate_text_mask(); }

	m_foreground_host = drawing_api::convert_mask(
		static_cast<uint32_t>(m_current_size.width),
		static_cast<uint32_t>(m_current_size.height),
		m_rendered_text,
		m_margin
	);

	m_dirty_bits &= ~host_textures_dirty;
	m_dirty_bits |= gpu_textures_dirty;
}

terraformer::scaling terraformer::ui::widgets::label::compute_size(main::widget_width_request)
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

terraformer::scaling terraformer::ui::widgets::label::compute_size(main::widget_height_request)
{
// TODO: Use width to find required height (multi-line)
	if(m_dirty_bits & text_dirty)
	{ regenerate_text_mask(); }

	return scaling{
		static_cast<float>(m_rendered_text.width() + 2*m_margin),
		static_cast<float>(m_rendered_text.height() + 2*m_margin),
		1.0f
	};
}

void terraformer::ui::widgets::label::theme_updated(main::config const& cfg, main::widget_instance_info)
{
	m_margin = static_cast<uint32_t>(cfg.output_area.padding + cfg.output_area.border_thickness);
	m_font = cfg.output_area.font;
	m_dirty_bits |= host_textures_dirty | text_dirty;
	m_fg_tint = cfg.output_area.colors.foreground;
	m_null_texture = cfg.misc_textures.null;
}


void terraformer::ui::widgets::label::prepare_for_presentation(main::widget_rendering_result output_rect)
{
	if(m_dirty_bits & host_textures_dirty) [[unlikely]]
	{ regenerate_textures(); }

	std::array const fg_tints{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint};
	if(output_rect.set_widget_foreground(m_foreground.get(), fg_tints) != main::set_texture_result::success) [[unlikely]]
	{
		m_foreground = output_rect.create_texture();
		output_rect.set_widget_foreground(m_foreground.get(), fg_tints);
		m_dirty_bits |= gpu_textures_dirty;
	}

	if(m_dirty_bits & gpu_textures_dirty)
	{
		m_foreground.upload(std::as_const(m_foreground_host).pixels());
		m_dirty_bits &= ~gpu_textures_dirty;
	}

	output_rect.set_widget_background(m_null_texture.get(), std::array<rgba_pixel, 4>{});
	output_rect.set_bg_layer_mask(m_null_texture.get());
	output_rect.set_selection_background(m_null_texture.get(), std::array<rgba_pixel, 4>{});	output_rect.set_frame(m_null_texture.get(), std::array<rgba_pixel, 4>{});
}
