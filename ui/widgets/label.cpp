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

terraformer::ui::main::widget_size_constraints terraformer::ui::widgets::label::compute_size_constraints()
{
	if(m_dirty_bits & text_dirty)
	{ regenerate_text_mask(); }

	return main::widget_size_constraints{
		.width = main::widget_size_range{
			static_cast<float>(m_rendered_text.width() + 2*m_margin),
			std::numeric_limits<float>::infinity()
		},
		.height = main::widget_size_range{
			static_cast<float>(m_rendered_text.height() + 2*m_margin),
			std::numeric_limits<float>::infinity()
		},
		.aspect_ratio = std::nullopt
	};
}

void terraformer::ui::widgets::label::theme_updated(main::config const& cfg, main::widget_instance_info)
{
	m_margin = static_cast<uint32_t>(cfg.output_area.padding + cfg.output_area.border_thickness);
	m_font = cfg.output_area.font;
	m_dirty_bits |= host_textures_dirty | text_dirty;
	m_fg_tint = cfg.output_area.colors.foreground;
	m_background = cfg.misc_textures.null;
}


void terraformer::ui::widgets::label::prepare_for_presentation(main::widget_rendering_result output_rect)
{
	if(m_dirty_bits & host_textures_dirty) [[unlikely]]
	{ regenerate_textures(); }

	if(output_rect.set_foreground(m_foreground.get()) != main::set_texture_result::success) [[unlikely]]
	{
		m_foreground = output_rect.create_texture();
		output_rect.set_foreground(m_foreground.get());
		m_dirty_bits |= gpu_textures_dirty;
	}

	if(m_dirty_bits & gpu_textures_dirty)
	{
		m_foreground.upload(std::as_const(m_foreground_host).pixels());
		m_dirty_bits &= ~gpu_textures_dirty;
	}

	output_rect.set_foreground_tints(std::array{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint});
	output_rect.set_background(m_background.get());
}
