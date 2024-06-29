//@	{"target":{"name":"button.o"}}

#include "./button.hpp"

void terraformer::ui::widgets::button::regenerate_text_mask()
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
	m_dirty_bits |= host_textures_dirty;
}

void terraformer::ui::widgets::button::regenerate_textures()
{
	if(m_dirty_bits & text_dirty) [[unlikely]]
	{ regenerate_text_mask(); }

	auto const background_intensity = m_background_intensity;
	{
		auto const val = background_intensity;
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
		auto const val = background_intensity;
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

	m_foreground_host = drawing_api::convert_mask(
		static_cast<uint32_t>(m_current_size.width),
		static_cast<uint32_t>(m_current_size.height),
		m_rendered_text,
		m_margin
	);

	m_dirty_bits &= ~host_textures_dirty;
}

terraformer::ui::main::widget_size_constraints terraformer::ui::widgets::button::get_size_constraints(
	object_dict const&) const
{
	if(m_dirty_bits & text_dirty)
	{ const_cast<button*>(this)->regenerate_text_mask(); }

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

void terraformer::ui::widgets::button::theme_updated(object_dict const& render_resources)
{
	auto const ui = render_resources/"ui";
	auto const margin = (ui/"widget_inner_margin").get_if<unsigned int const>();
	auto const border_thickness = (ui/"3d_border_thickness").get_if<unsigned int const>();

	assert(margin != nullptr);
	assert(border_thickness != nullptr);
	m_margin = *margin + *border_thickness;
	m_border_thickness = *border_thickness;

	auto const command_area = ui/"command_area";
	assert(!command_area.is_null());
	m_font = command_area.dup("font");
	assert(m_font);
	auto const background_intensity = (command_area/"background_intensity").get_if<float const>();
	assert(background_intensity != nullptr);
	m_background_intensity = *background_intensity;
	m_dirty_bits |= host_textures_dirty | text_dirty;
}
