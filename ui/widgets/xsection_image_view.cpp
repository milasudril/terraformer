//@	{"target":{"name":"xsection_image_view.o"}}

#include "./xsection_image_view.hpp"

#include "lib/common/spaces.hpp"
#include "lib/math_utils/interp.hpp"

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

	m_src_image_box_xy = box_size{
		static_cast<float>(m_source_image.width()),
		static_cast<float>(m_source_image.height()),
		0.0f
	};

	update_src_image_box_xz();
}

namespace
{
	struct draw_xsections_params
	{
		terraformer::box_size output_size;
		float xy_scale;
		float z_min;
		float z_max;
	};

	terraformer::image draw_cross_sections(
		terraformer::span_2d<float const> input,
		draw_xsections_params const& params
	)
	{
		auto const w = static_cast<uint32_t>(params.output_size[0] + 0.5f);
		auto const h = static_cast<uint32_t>(params.output_size[1] + 0.5f);
		assert(h >= 1);
		assert(w >= 1);

		terraformer::image ret{w, h};
		auto const dy = static_cast<float>(input.height())/16.0f;

		for(size_t k = 0; k != 16; ++k)
		{
			auto const slice_offset = dy*(static_cast<float>(k) + 0.5f);
			for(uint32_t x_out = 0; x_out != w; ++x_out)
			{
				auto const x_in = static_cast<float>(x_out)*params.xy_scale;
				auto const y_in = slice_offset;
				auto const z_in = interp(input, x_in, y_in, terraformer::clamp_at_boundary{});
				auto const z_out = static_cast<float>(h)*(params.z_max - z_in)/
					(params.z_max - params.z_min);
				ret(x_out, std::min(static_cast<uint32_t>(z_out), h - 1)) =
					terraformer::rgba_pixel{0.0f, 0.0f, 0.0f, 1.0f};
			}
		}
		return ret;
	}
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

	if(m_redraw_required)
	{
		m_diagram = draw_cross_sections(
			m_source_image.pixels(),
			draw_xsections_params{
				.output_size = m_adjusted_box,
				.xy_scale = m_src_image_box_xy[0]/m_adjusted_box[0],
				.z_min = m_min_val,
				.z_max = m_max_val
			}
		);
		m_redraw_required = false;
	}

	std::array const fg_tints{
		rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
		rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
		rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
		rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f}
	};

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
			.texture = m_diagram.get_backend_resource(backend).get(),
			.tints = fg_tints
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
