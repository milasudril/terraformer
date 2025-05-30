//@	{"target":{"name":"button.o"}}

#include "./button.hpp"

#include "ui/drawing_api/image_generators.hpp"

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

	m_background_released = generate(
		drawing_api::beveled_rectangle{
			.domain_size = span_2d_extents{
				.width = static_cast<uint32_t>(m_current_size.width),
				.height = static_cast<uint32_t>(m_current_size.height),
			},
			.origin_x = 0,
			.origin_y = 0,
			.width = static_cast<uint32_t>(m_current_size.width),
			.height = static_cast<uint32_t>(m_current_size.height),
			.border_thickness = m_border_thickness,
			.upper_left_color = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
			.lower_right_color = rgba_pixel{0.25f, 0.25f, 0.25f, 1.0f},
			.fill_color = rgba_pixel{0.5f, 0.5f, 0.5f, 1.0f}
		}
	);

	m_background_pressed = generate(
		drawing_api::beveled_rectangle{
			.domain_size = span_2d_extents{
				.width = static_cast<uint32_t>(m_current_size.width),
				.height = static_cast<uint32_t>(m_current_size.height),
			},
			.origin_x = 0,
			.origin_y = 0,
			.width = static_cast<uint32_t>(m_current_size.width),
			.height = static_cast<uint32_t>(m_current_size.height),
			.border_thickness = m_border_thickness,
			.upper_left_color = rgba_pixel{0.25f, 0.25f, 0.25f, 1.0f},
			.lower_right_color = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
			.fill_color = rgba_pixel{0.5f, 0.5f, 0.5f, 1.0f},
		}
	);

	m_foreground = drawing_api::convert_mask(m_rendered_text);

	m_dirty_bits &= ~host_textures_dirty;
}

terraformer::ui::main::widget_layer_stack
terraformer::ui::widgets::button::prepare_for_presentation(main::graphics_backend_ref backend)
{
	auto const display_state = m_value ? state::pressed : m_state_to_display;

	if(m_dirty_bits & host_textures_dirty) [[unlikely]]
	{ regenerate_textures(); }

	auto const null_texture = m_null_texture->get_backend_resource(backend).get();

	return main::widget_layer_stack{
		.background = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = (display_state == state::released)?
				m_background_released.get_backend_resource(backend).get():
				m_background_pressed.get_backend_resource(backend).get(),
			.tints = std::array{m_bg_tint, m_bg_tint, m_bg_tint, m_bg_tint}
		},
		.sel_bg_mask = main::widget_layer_mask{
			.offset = displacement{},
			.texture = null_texture
		},
		.selection_background = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		},
		.foreground = main::widget_layer{
			.offset = displacement{m_margin, m_margin, 0.0f},
			.rotation = geosimd::turn_angle{},
			.texture = m_foreground.get_backend_resource(backend).get(),
			.tints = std::array{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint}
		},
		.frame = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		},
		.input_marker = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		}
	};
}

terraformer::box_size terraformer::ui::widgets::button::compute_size(main::widget_width_request)
{
	// TODO: Use height to find required width (multi-line)
	if(m_dirty_bits & text_dirty)
	{ regenerate_text_mask(); }

	return box_size{
		static_cast<float>(m_rendered_text.width()) + 2.0f*m_margin,
		static_cast<float>(m_rendered_text.height()) + 2.0f*m_margin,
		0.0f
	};
}

terraformer::box_size terraformer::ui::widgets::button::compute_size(main::widget_height_request)
{
	// TODO: Use width to find required height (multi-line)
	if(m_dirty_bits & text_dirty)
	{ regenerate_text_mask(); }

	return box_size{
		static_cast<float>(m_rendered_text.width()) + 2.0f*m_margin,
		static_cast<float>(m_rendered_text.height()) + 2.0f*m_margin,
		0.0f
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
