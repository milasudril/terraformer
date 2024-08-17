//@	{"target":{"name":"label.o"}}

#include "./label.hpp"

void terraformer::ui::widgets::label::regenerate_text_mask()
{
	auto const font = m_font.get_if<font_handling::font const>();
	assert(font != nullptr);

	font_handling::text_shaper shaper{};

	// TODO: Add support for different scripts, direction, and languages
	auto result = shaper.append(m_text)
		.with(hb_script_t::HB_SCRIPT_LATIN)
		.with(hb_direction_t::HB_DIRECTION_LTR)
		.with(hb_language_from_string("en-UE", -1))
		.run(*font);

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

terraformer::ui::main::widget_size_constraints terraformer::ui::widgets::label::update_geometry()
{
	if(m_dirty_bits & text_dirty)
	{ regenerate_text_mask(); }

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

void terraformer::ui::widgets::label::theme_updated(object_dict const& render_resources)
{
	auto const ui = render_resources/"ui";
	assert(!ui.is_null());
	auto const margin = (ui/"widget_inner_margin").get_if<unsigned int const>();
	auto const border_thickness = (ui/"3d_border_thickness").get_if<unsigned int const>();

	assert(margin != nullptr);
	assert(border_thickness != nullptr);
	m_margin = *margin + *border_thickness;
	m_border_thickness = *border_thickness;
	m_background = ui.dup("null_texture");
	assert(m_background);

	auto const output_area = ui/"output_area";
	assert(!output_area.is_null());
	m_font = output_area.dup("font");
	assert(m_font);
	m_dirty_bits |= host_textures_dirty | text_dirty;
	auto const fg_tint = (output_area/"text_color").get_if<rgba_pixel const>();
	assert(fg_tint != nullptr);
	m_fg_tint = *fg_tint;
}
