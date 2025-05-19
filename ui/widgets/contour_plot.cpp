//@	{"target": {"name":"contour_plot.o", "rel":"implementation"}}

#include "./contour_plot.hpp"
#include "ui/drawing_api/image_generators.hpp"

void terraformer::ui::widgets::contour_plot::show_image(span_2d<float const> input_image)
{
	auto const w = input_image.width();
	auto const h = input_image.height();

	grayscale_image img_posterized{w, h};
	auto const dz = m_dz;

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{ img_posterized(x, y) = std::round(input_image(x, y)/dz); }
	}

	image output_image{w, h};
	assert(h > 2);
	assert(w > 2);

	for(uint32_t y = 1; y != h - 1; ++y)
	{
		for(uint32_t x = 1; x != w - 1; ++x)
		{
			auto const ddx = img_posterized(x + 1, y) - img_posterized(x - 1, y);
			auto const ddy = img_posterized(x, y + 1) - img_posterized(x, y - 1);
			auto const val = std::abs(ddx) + std::abs(ddy) > 0.0f;
			output_image(x, y) = val*rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f};
		}
	}


	auto output_frame = generate(
		drawing_api::flat_rectangle{
			.domain_size = span_2d_extents {
				.width = w,
				.height = h
			},
			.origin_x = 0u,
			.origin_y = 0u,
			.width = w,
			.height = h,
			.border_thickness = m_border_thickness,
			.border_color = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
			.fill_color = rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f}
		}
	);

	std::lock_guard lock{m_image_mutex};
	m_image = std::move(output_image);
	m_frame = std::move(output_frame);
}

terraformer::ui::main::widget_layer_stack terraformer::ui::widgets::contour_plot::prepare_for_presentation(main::graphics_backend_ref backend)
{
	auto const null_texture = m_null_texture->get_backend_resource(backend).get();

	std::lock_guard lock{m_image_mutex};
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
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = m_image.get_backend_resource(backend).get(),
			.tints = std::array{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint},
		},
		.frame = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = m_frame.get_backend_resource(backend).get(),
			.tints = std::array{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint}
		},
		.input_marker{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		}
	};
}