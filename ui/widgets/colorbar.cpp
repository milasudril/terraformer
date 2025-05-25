//@	{"target": {"name":"colorbar.o", "rel":"implementation"}}

#include "./colorbar.hpp"
#include "ui/drawing_api/image_generators.hpp"
#include "lib/common/utils.hpp"

#include <format>

void terraformer::ui::widgets::colorbar::update_colorbar()
{
	auto const w = static_cast<uint32_t>(m_size[0] + 0.5f);
	auto const h = static_cast<uint32_t>(m_size[1] + 0.5f);
	image output_image{w, h};

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const grayscale_val = static_cast<float>(h - y - 1)/(static_cast<float>(h - 1));
			output_image(x, y) = m_color_map(grayscale_val);
		}
	}
	m_image = std::move(output_image);
}

void terraformer::ui::widgets::colorbar::update_frame()
{
	auto const w = static_cast<uint32_t>(m_size[0] + 0.5f);
	auto const h = static_cast<uint32_t>(m_size[1] + 0.5f);
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

	for(uint32_t index = 0; index != 14; ++index)
	{
		auto const intensity = static_cast<float>(index)/13;
		auto const y = static_cast<uint32_t>((1.0f - intensity)*static_cast<float>(h - 1) + 0.5f);
		for(uint32_t x = 0; x != w; ++x)
		{
			if(x < w/4 || x >= w - w/4)
			{ output_frame(x, y) = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f}; }
		}
	}

	m_frame = std::move(output_frame);
}

terraformer::ui::main::widget_layer_stack terraformer::ui::widgets::colorbar::prepare_for_presentation(main::graphics_backend_ref backend)
{
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
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = m_image.get_backend_resource(backend).get(),
			.tints = std::array<rgba_pixel, 4>{
				rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
				rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
				rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
				rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f}
			}
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