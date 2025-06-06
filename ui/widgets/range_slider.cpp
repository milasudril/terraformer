//@	{"target":{"name":"range_slider.o"}}

#include "./range_slider.hpp"

#include "lib/common/spaces.hpp"
#include "ui/drawing_api/image_generators.hpp"
#include "ui/main/widget.hpp"

void terraformer::ui::widgets::range_slider::regenerate_selection_mask()
{
	auto const length = m_current_range.max() - m_current_range.min();
	auto const bt = static_cast<uint32_t>(m_border_thickness);
	// TODO: Selection mask should be grayscale

	if(m_orientation == main::widget_orientation::horizontal)
	{
		auto const track_length = static_cast<float>(m_current_size.width - bt);
		auto const sel_begin = m_current_range.min()*track_length + m_border_thickness;

		m_selection_mask = generate(
			drawing_api::flat_rectangle{
				.domain_size = span_2d_extents {
					.width = static_cast<uint32_t>(m_current_size.width),
					.height = static_cast<uint32_t>(m_current_size.height)
				},
				.origin_x = static_cast<uint32_t>(sel_begin),
				.origin_y = bt,
				.width = std::max(static_cast<uint32_t>(track_length*length + 0.5f), 1u),
				.height = static_cast<uint32_t>(m_current_size.height) - 2*bt,
				.border_thickness = 0u,
				.border_color = rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
				.fill_color = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f}
			}
		);
	}
	else
	{
		auto const track_length = static_cast<float>(m_current_size.height - bt);
		auto const sel_begin = m_current_range.min()*track_length + m_border_thickness;

		m_selection_mask = generate(
			drawing_api::flat_rectangle{
				.domain_size = span_2d_extents {
					.width = static_cast<uint32_t>(m_current_size.width),
					.height = static_cast<uint32_t>(m_current_size.height)
				},
				.origin_x = bt,
				.origin_y = static_cast<uint32_t>(sel_begin),
				.width = static_cast<uint32_t>(m_current_size.width) - 2*bt,
				.height = std::max(static_cast<uint32_t>(track_length*length + 0.5f), 1u),
				.border_thickness = 0u,
				.border_color = rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f},
				.fill_color = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f}
			}
		);
	}
	m_dirty_bits &= ~selection_dirty;
}

void terraformer::ui::widgets::range_slider::regenerate_track()
{
	m_frame = generate(
		drawing_api::flat_rectangle{
			.domain_size = span_2d_extents {
				.width = static_cast<uint32_t>(m_current_size.width),
				.height = static_cast<uint32_t>(m_current_size.height)
			},
			.origin_x = 0u,
			.origin_y = 0u,
			.width = static_cast<uint32_t>(m_current_size.width),
			.height = static_cast<uint32_t>(m_current_size.height),
			.border_thickness = static_cast<uint32_t>(m_border_thickness),
			.border_color = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
			.fill_color = rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f}
		}
	);

	m_dirty_bits &= ~track_dirty;
}

terraformer::ui::main::widget_layer_stack
terraformer::ui::widgets::range_slider::prepare_for_presentation(main::graphics_backend_ref backend)
{
	if(m_dirty_bits & track_dirty) [[unlikely]]
	{ regenerate_track(); }

	if(m_dirty_bits & selection_dirty)
	{ regenerate_selection_mask(); }

	auto const null_texture = m_null_texture->get_backend_resource(backend).get();

	auto const bg = m_background->get_backend_resource(backend).get();

	return main::widget_layer_stack{
		.background = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = bg,
			.tints = std::array{m_bg_tint, m_bg_tint, m_bg_tint, m_bg_tint}
		},
		.sel_bg_mask = main::widget_layer_mask{
			.offset = displacement{},
			.texture = m_selection_mask.get_backend_resource(backend).get()
		},
		.selection_background = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = bg,
			.tints = std::array{m_sel_tint, m_sel_tint, m_sel_tint, m_sel_tint}
		},
		.foreground = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		},
		.frame = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = m_frame.get_backend_resource(backend).get(),
			.tints = std::array{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint}
		},
		.input_marker = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		}
	};
}

terraformer::box_size terraformer::ui::widgets::range_slider::compute_size(main::widget_width_request)
{
	auto const short_side = 1.5f*static_cast<float>(m_track_size);
	auto const long_side = short_side*4.0f*3.0f;

	if(m_orientation == main::widget_orientation::horizontal)
	{
		return box_size{long_side, short_side, 0.0f}
			+ 2.0f*displacement{m_border_thickness, m_border_thickness, 0.0f};
	}

	return box_size{long_side, short_side, 0.0f}
		+ 2.0f*displacement{m_border_thickness, m_border_thickness, 0.0f};
}

terraformer::box_size terraformer::ui::widgets::range_slider::compute_size(main::widget_height_request)
{
	auto const short_side = 1.5f*static_cast<float>(m_track_size);
	auto const long_side = short_side*4.0f*3.0f;

	if(m_orientation == main::widget_orientation::horizontal)
	{
		return box_size{long_side, short_side, 0.0f}
			+ 2.0f*displacement{m_border_thickness, m_border_thickness, 0.0f};
	}

	return box_size{long_side, short_side, 0.0f}
		+ 2.0f*displacement{m_border_thickness, m_border_thickness, 0.0f};
}

void terraformer::ui::widgets::range_slider::theme_updated(main::config const& cfg, main::widget_instance_info)
{
	m_bg_tint = cfg.input_area.colors.background;
	m_fg_tint = cfg.input_area.colors.foreground;
	m_sel_tint = cfg.input_area.colors.selection;
	m_border_thickness = static_cast<uint32_t>(cfg.input_area.border_thickness);
	m_null_texture = cfg.misc_textures.null;
	m_background = cfg.misc_textures.white;
	m_track_size = cfg.input_area.font->get_font_size();

	m_dirty_bits |= track_dirty | selection_dirty;
}
