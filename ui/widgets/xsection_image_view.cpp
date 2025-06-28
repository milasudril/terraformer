//@	{"target":{"name":"xsection_image_view.o"}}

#include "./xsection_image_view.hpp"

void terraformer::ui::widgets::xsection_image_view::show_image(span_2d<float const> image)
{
	m_source_image = grayscale_image{image};
	m_redraw_required = true;

	auto const size = static_cast<size_t>(image.width())*static_cast<size_t>(image.height());
	auto const minmax = std::ranges::minmax_element(image.data(), image.data() + size);
	auto const min = *minmax.min;
	auto const max = *minmax.max;

	m_min_val = min > 0.0f? 0.0f : min;
	m_max_val = max < 0.0f? 0.0f : max;
}

terraformer::ui::main::widget_layer_stack terraformer::ui::widgets::xsection_image_view::prepare_for_presentation(main::graphics_backend_ref backend)
{
	std::array const bg_tints{
		m_cfg.bg_tint,
		m_cfg.bg_tint,
		m_cfg.bg_tint,
		m_cfg.bg_tint
	};
	auto const null_texture = m_cfg.null_texture->get_backend_resource(backend).get();
	auto const background = m_cfg.background->get_backend_resource(backend).get();
	return main::widget_layer_stack{
		.background = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = background,
			.tints = bg_tints
		},
		.sel_bg_mask = main::widget_layer_mask{
			.offset = displacement{},
			.texture = null_texture,
		},
		.selection_background = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		},
		.foreground = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			// TODO: This should be the image to display
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		},
		.frame = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			// TODO: Will be used to add a frame around the image
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
