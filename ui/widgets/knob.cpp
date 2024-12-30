//@	{"target":{"name":"knob.o"}}

#include "./knob.hpp"

#include "ui/drawing_api/image_generators.hpp"

terraformer::ui::main::widget_layer_stack
terraformer::ui::widgets::knob::prepare_for_presentation(main::graphics_backend_ref backend)
{
	auto const null_texture = m_null_texture->get_backend_resource(backend).get();
//	auto const val = internal_value();
	return main::widget_layer_stack{
		.background = main::widget_layer{
			.offset = displacement{},
			.texture = m_handle->get_backend_resource(backend).get(),
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
			.offset = displacement{},
			.texture = m_hand->get_backend_resource(backend).get(),
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

terraformer::scaling terraformer::ui::widgets::knob::compute_size(main::widget_width_request)
{
	auto const w = static_cast<float>(m_handle->frontend_resource().width());
	auto const h = static_cast<float>(m_handle->frontend_resource().height());
	return scaling{w, h, 1.0f};
}

terraformer::scaling terraformer::ui::widgets::knob::compute_size(main::widget_height_request)
{
	auto const w = static_cast<float>(m_handle->frontend_resource().width());
	auto const h = static_cast<float>(m_handle->frontend_resource().height());
	return scaling{w, h, 1.0f};
}

void terraformer::ui::widgets::knob::theme_updated(main::config const& cfg, main::widget_instance_info)
{
	m_bg_tint = cfg.command_area.colors.background;
	m_fg_tint = cfg.command_area.colors.foreground;
	m_null_texture = cfg.misc_textures.null;
	m_handle = cfg.misc_textures.small_knob;
	m_hand = cfg.misc_textures.small_hand;
}
