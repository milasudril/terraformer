//@	{"target":{"name":"slider.o"}}

#include "./slider.hpp"

#include "ui/drawing_api/image_generators.hpp"

void terraformer::ui::widgets::slider::regenerate_textures()
{	
	m_handle = generate(
		drawing_api::beveled_rectangle{
			.domain_size = span_2d_extents{
				.width = static_cast<uint32_t>(m_current_size.width),
				.height = static_cast<uint32_t>(m_current_size.height),
			},
			.width = static_cast<uint32_t>(m_current_size.width),
			.height = static_cast<uint32_t>(m_current_size.height),
			.border_thickness = m_border_thickness,
			.upper_left_color = m_bg_tint,
			.lower_right_color = 0.25f*m_bg_tint + rgba_pixel{0.0f, 0.0f, 0.0f, 0.75f},
			.fill_color = 0.5f*m_bg_tint + rgba_pixel{0.0f, 0.0f, 0.0f, 0.5f},
		}
	);

	m_dirty_bits &= ~handle_dirty;

	m_track = generate(
		drawing_api::beveled_rectangle{
			.domain_size = span_2d_extents{
				.width = static_cast<uint32_t>(m_current_size.width),
				.height = static_cast<uint32_t>(m_current_size.height),
			},
			.width = (m_orientation == orientation::horizontal)?
				static_cast<uint32_t>(m_current_size.width):
				2*m_border_thickness,
			.height = (m_orientation == orientation::vertical)?
				static_cast<uint32_t>(m_current_size.height):
				2*m_border_thickness,
			.border_thickness = m_border_thickness,
			.upper_left_color = 0.25f*m_bg_tint + rgba_pixel{0.0f, 0.0f, 0.0f, 0.75f},
			.lower_right_color = m_bg_tint,
			.fill_color = 0.5f*m_bg_tint + rgba_pixel{0.0f, 0.0f, 0.0f, 0.5f},
		}
	);

	m_dirty_bits &= ~track_dirty;
}

terraformer::ui::main::widget_layer_stack
terraformer::ui::widgets::slider::prepare_for_presentation(main::graphics_backend_ref backend)
{
	if(m_dirty_bits & (handle_dirty|track_dirty)) [[unlikely]]
	{ regenerate_textures(); }

	auto const null_texture = m_null_texture->get_backend_resource(backend).get();

	return main::widget_layer_stack{
		.background = main::widget_layer{
			.offset = displacement{},
			.texture = m_track.get_backend_resource(backend).get(),
			.tints = std::array{m_bg_tint, m_bg_tint, m_bg_tint, m_bg_tint}
		},
		.sel_bg_mask = main::widget_layer_mask{
			.offset = displacement{},
			.texture = null_texture
		},
		.selection_background = main::widget_layer{
			.offset = displacement{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		},
		.foreground = main::widget_layer{
			.offset = displacement{},  // TODO: Derive offset from current value
			.texture = null_texture, //m_handle.get_backend_resource(backend).get(),
			.tints = std::array{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint}
		},
		.frame = main::widget_layer{
			.offset = displacement{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		},
		// TODO: Use this layer for tick marks
		.input_marker = main::widget_layer{
			.offset = displacement{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		}
	};
}

terraformer::scaling terraformer::ui::widgets::slider::compute_size(main::widget_width_request)
{
	// TODO: Express minimum size in terms of handle size
	return scaling{2.0f*m_margin, 2.0f*m_margin, 1.0f};
}

terraformer::scaling terraformer::ui::widgets::slider::compute_size(main::widget_height_request)
{
return scaling{2.0f*m_margin, 2.0f*m_margin, 1.0f};
}

void terraformer::ui::widgets::slider::theme_updated(main::config const& cfg, main::widget_instance_info)
{
	m_margin = cfg.command_area.padding + cfg.command_area.border_thickness;
	m_font = cfg.command_area.font;
	m_bg_tint = cfg.command_area.colors.background;
	m_fg_tint = cfg.command_area.colors.foreground;
	m_border_thickness = static_cast<uint32_t>(cfg.command_area.border_thickness);
	m_dirty_bits |= track_dirty | handle_dirty;
	m_null_texture = cfg.misc_textures.null;
}
