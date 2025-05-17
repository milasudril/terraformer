//@	{"target":{"name":"label.o"}}

#include "./label.hpp"
#include "ui/drawing_api/image_generators.hpp"

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

	m_foreground = drawing_api::convert_mask(m_rendered_text);

	m_dirty_bits &= ~host_textures_dirty;
}

terraformer::box_size terraformer::ui::widgets::label::compute_size(main::widget_width_request)
{
	// TODO: Use height to find required width (multi-line)
	if(m_dirty_bits & text_dirty)
	{ regenerate_text_mask(); }

	return box_size{
		static_cast<float>(m_rendered_text.width()),
		static_cast<float>(m_rendered_text.height()),
		0.0f
	} + m_margin*displacement{2.0f, 2.0f, 0.0f};
}

terraformer::box_size terraformer::ui::widgets::label::compute_size(main::widget_height_request)
{
// TODO: Use width to find required height (multi-line)
	if(m_dirty_bits & text_dirty)
	{ regenerate_text_mask(); }

	return box_size{
		static_cast<float>(m_rendered_text.width()),
		static_cast<float>(m_rendered_text.height()),
		0.0f
	} + m_margin*displacement{2.0f, 2.0f, 0.0f};
}

void terraformer::ui::widgets::label::theme_updated(main::config const& cfg, main::widget_instance_info)
{
	m_margin = cfg.output_area.padding + cfg.output_area.border_thickness;
	m_font = cfg.output_area.font;
	m_dirty_bits |= host_textures_dirty | text_dirty;
	m_fg_tint = cfg.output_area.colors.foreground;
	m_null_texture = cfg.misc_textures.null;
}

terraformer::ui::main::widget_layer_stack
terraformer::ui::widgets::label::prepare_for_presentation(main::graphics_backend_ref backend)
{
	if(m_dirty_bits & host_textures_dirty) [[unlikely]]
	{ regenerate_textures(); }

	auto const null_texture = m_null_texture->get_backend_resource(backend).get();

	return main::widget_layer_stack{
		.background = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
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
			.tints = std::array<rgba_pixel, 4>{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint}
		},
		.frame = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		},
		.input_marker{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		}
	};
}
