//@	{"target": {"name":"colorbar.o", "rel":"implementation"}}

#include "./colorbar.hpp"
#include "ui/drawing_api/image_generators.hpp"
#include "lib/common/string_converter.hpp"

#include <format>

void terraformer::ui::widgets::colorbar::update_colorbar()
{
	image output_image{32, 512};

	auto const h = output_image.height();
	auto const w = output_image.width();

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
	constexpr auto h = 512;
	constexpr auto w  = 32;
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

	auto const value_map_ptr = m_value_map.get().get_pointer();
	auto const from_value = m_value_map.get().get_vtable().from_value;
	auto const to_value = m_value_map.get().get_vtable().to_value;

	for(uint32_t index = 0; index != 13; ++index)
	{
		auto const intensity = static_cast<float>(index)/12;
		auto const value = to_value(value_map_ptr, intensity);
		auto const value_string = std::format("{:.1e}", value);
		// TODO: Render value string
		// NOTE: value_map_ptr is not used to produce the gradient => Create a separate label generator function
		auto const printed_value = num_string_converter<float>{}.convert(value_string);
		auto const actual_intensity = from_value(value_map_ptr, printed_value);

		auto const y = static_cast<uint32_t>((1.0f - actual_intensity)*(h - 1) + 0.5f);
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