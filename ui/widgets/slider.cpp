//@	{"target":{"name":"slider.o"}}

#include "./slider.hpp"

#include "ui/drawing_api/image_generators.hpp"

void terraformer::ui::widgets::slider::regenerate_textures()
{	
	auto const margin = static_cast<uint32_t>(track_margin());
	auto const orientation = m_orientation;
	m_track = generate(
		drawing_api::beveled_rectangle{
			.domain_size = span_2d_extents{
				.width = static_cast<uint32_t>(m_current_size.width),
				.height = static_cast<uint32_t>(m_current_size.height)
			},
			// TODO: This will break when tick marks are added
			.origin_x = orientation == orientation::horizontal?
				margin :
				m_current_size.width/2 - m_border_thickness,
			.origin_y = orientation == orientation::vertical? 
				margin : 
				m_current_size.height/2 - m_border_thickness,
			.width = (orientation == orientation::horizontal)?
				static_cast<uint32_t>(track_length()):
				2*m_border_thickness,
			.height = (orientation == orientation::vertical)?
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
	printf("Render %.8g\n", m_value);
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
			.offset = displacement{
				m_value*track_length(),
				0.0f,
				0.0f
			},  // TODO: Derive offset from current value
			.texture = m_handle->get_backend_resource(backend).get(),
			.tints = std::array{m_bg_tint, m_bg_tint, m_bg_tint, m_bg_tint}
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
	auto const w = static_cast<float>(m_handle->frontend_resource().width());
	auto const h = static_cast<float>(m_handle->frontend_resource().height());
	return scaling{2.0f*track_margin() + 16.0f*w, h, 1.0f};
}

terraformer::scaling terraformer::ui::widgets::slider::compute_size(main::widget_height_request)
{
	auto const w = static_cast<float>(m_handle->frontend_resource().width());
	auto const h = static_cast<float>(m_handle->frontend_resource().height());
	return scaling{2.0f*track_margin() + 16.0f*w, h, 1.0f};
}

void terraformer::ui::widgets::slider::theme_updated(main::config const& cfg, main::widget_instance_info)
{
	m_font = cfg.command_area.font;
	m_bg_tint = cfg.command_area.colors.background;
	m_fg_tint = cfg.command_area.colors.foreground;
	m_border_thickness = static_cast<uint32_t>(cfg.command_area.border_thickness);
	m_dirty_bits |= track_dirty;
	m_null_texture = cfg.misc_textures.null;
	m_handle = cfg.misc_textures.vertical_handle;
}
